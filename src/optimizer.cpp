//-----------------------------
// intermediate code optimizer
//-----------------------------

#include "optimizer.h"
#include "function.h"
#include "main.h"
#include "scheduler.h"
#include "logger.h"

#include <proto/dos.h>


//---------------------------------------------------------------------------------------------

void Edge::print()
{
	char buf[24];

	Printf("Edge %ld, range [%ld, %ld] operand %s.\n", eIndex, startInstruction, endInstruction,
		tip.makeString(buf));
}

//---------------------------------------------------------------------------------------------

void Optimizer::dumpEdges()
{
	PutStr("---- edges ----\n");
	for (Edge *e = edges.first(); e; e = e->next()) e->print();
	PutStr("----------------\n");
}

//---------------------------------------------------------------------------------------------

bool Optimizer::optimizeFunction()
{
	bool success = FALSE;
	
	Printf("optimizing %s().\n", f->name);
	convertToEdges();
	f->print();
	fuseImmediateOperands();
	f->print();
	assignRegistersToArguments();
	f->print();
	updateEdgesIntervals();
	dumpEdges();

	if (success = allocateRegisters())
	{
		dumpEdges();
		applyRegistersToCode();
		killRedundantMoves();
	}
		
	return success;
}

//---------------------------------------------------------------------------------------------

Edge* Optimizer::findEdgeByTip(Operand &op)
{
	for (Edge *e = edges.first(); e; e = e->next())
	{
		if (e->tipIs(op)) return e;
	}

	return NULL;
}

//---------------------------------------------------------------------------------------------

Edge* Optimizer::findEdgeByIndex(int index)
{
	for (Edge *e = edges.first(); e; e = e->next())
	{
		if (e->index() == index) return e;
	}

	return NULL;
}

//---------------------------------------------------------------------------------------------

Edge* Optimizer::addEdge(Operand &start)
{
	Edge *e;

	if (e = new Edge(edgeCount, start))
	{
		edgeCount++;
		edges.addTail(e);
	}

	return e;
}


//---------------------------------------------------------------------------------------------
// At least one of MOVE operands is a virtual register.

void Optimizer::convertMoveToEdges(InterInstruction *ii)
{
	if (ii->out.type == IIOP_VIRTUAL)
	{
		switch (ii->arg.type)
		{
			case IIOP_FARGUMENT:
			case IIOP_CRESULT:
			case IIOP_ADDRREG:
			case IIOP_DATAREG:
				if (Edge *e = addEdge(ii->out))
				{
					ii->out.type = IIOP_EDGE;
					ii->out.value = e->index();
				}
			break;

			case IIOP_VIRTUAL:
				if (Edge *e = findEdgeByTip(ii->arg))
				{
					e->advanceTo(ii->out);
					ii->remove();
				}
			break;
		}
	}
	else if (ii->arg.type == IIOP_VIRTUAL)
	{
		switch (ii->out.type)
		{
			case IIOP_FRESULT:
			case IIOP_CARGUMENT:
			case IIOP_FRAME:
			case IIOP_ADDRREG:
			case IIOP_DATAREG:
				if (Edge *e = findEdgeByTip(ii->arg))
				{
					ii->arg.type = IIOP_EDGE;
					ii->arg.value = e->index();
					e->terminate();
				}
			break;
		}
	}
}

//---------------------------------------------------------------------------------------------
// DMOV moves external data to virtual register.

void Optimizer::convertDmovToEdges(InterInstruction *ii)
{
	switch (ii->arg.type)
	{
		case IIOP_IMMEDIATE:
			if (Edge *e = addEdge(ii->out))
			{
				ii->out.type = IIOP_EDGE;
				ii->out.value = e->index();
			}
			break;
		}
	}


//---------------------------------------------------------------------------------------------
// COPY is always from virtual register to virtual register. It creates a new edge with tip at
// destination operand.

void Optimizer::convertCopyToEdges(InterInstruction *ii)
{
	if (Edge *e1 = findEdgeByTip(ii->arg))
	{
		if (Edge *e2 = addEdge(ii->out))
		{
			ii->arg.type = IIOP_EDGE;
			ii->arg.value = e1->index();
			ii->out.type = IIOP_EDGE;
			ii->out.value = e2->index();
		}
	}
}

//---------------------------------------------------------------------------------------------
// Dyadic operation operands are always virtual registers. First operand edge is terminated.
// Second operand edge is continued.

void Optimizer::convertDyadicToEdges(InterInstruction *ii)
{
	Edge *side, *thru;

	side = findEdgeByTip(ii->arg);
	thru = findEdgeByTip(ii->out);

	if (side && thru)
	{
		ii->arg.type = IIOP_EDGE;
		ii->arg.value = side->index();
		side->terminate();
		ii->out.type = IIOP_EDGE;
		ii->out.value = thru->index();
	}
}

//---------------------------------------------------------------------------------------------

void Optimizer::convertToEdges()
{
	for (InterInstruction *ii = f->code.first(); ii; ii = ii->next())
	{
		switch (ii->code)
		{
			case II_MOVE: convertMoveToEdges(ii); break;
			case II_DMOV: convertDmovToEdges(ii); break;
			case II_COPY: convertCopyToEdges(ii); break;
			case II_ADDL:
			case II_SUBL: convertDyadicToEdges(ii); break; 
		}
	}
}

//---------------------------------------------------------------------------------------------
// Function iterares through instruction list. If it finds a DMOV with immediate source operand
// and edge destination, it follows the edge. If the edge ends at a source operand of a dyadic
// operation, immediate operand is fused to it. DMOV is removed, edge is removed too.

void Optimizer::fuseImmediateOperands()
{
	for (InterInstruction *ii = f->code.first(); ii; ii = ii->next())
	{
		if ((ii->code == II_DMOV) && (ii->arg.type == IIOP_IMMEDIATE) && (ii->out.type = IIOP_EDGE))
		{
			for (InterInstruction *ij = ii->next(); ij; ij = ij->next())
			{
				if (ij->isDyadic() && (ij->arg == ii->out))
				{
					ij->arg = ii->arg;
					ii->remove();
					findEdgeByIndex(ii->out.value)->remove();
				}
			}
		}
	}
} 

//---------------------------------------------------------------------------------------------
// m68k register assignment for function arguments and results. For now it is very simple.
// Fa0/Ca0 is assigned to d0, Fa1/Ca1 to d1... If there are more than 8 arguments, pseudo-
// registers from d8 are used. They are stored in memory, and addressed via a5 ('d8' is 0(a5),
// 'd9' is 4(a5) and so on). Results (Fr/Cr) are assigned in the same way. 

void Optimizer::assignRegistersToArguments()
{
	for (InterInstruction *ii = f->code.first(); ii; ii = ii->next())
	{
		if (ii->code == II_MOVE)
		{
			if ((ii->arg.type == IIOP_FARGUMENT) || (ii->arg.type == IIOP_CRESULT))
			{
				if (ii->arg.value < 8) ii->arg.type = IIOP_DATAREG;
				else ii->arg.type = IIOP_MEMREG;
			}
			else if ((ii->out.type == IIOP_CARGUMENT) || (ii->out.type == IIOP_FRESULT))
			{
				if (ii->out.value < 8) ii->out.type = IIOP_DATAREG;
				else ii->out.type = IIOP_MEMREG;
			}
		}
 	}
}

//---------------------------------------------------------------------------------------------

void Optimizer::updateEdgesIntervals()
{
	int iCounter = 0;

	for (InterInstruction *ii = f->code.first(); ii; ii = ii->next())
	{
		switch (ii->code)
		{
			case II_DMOV:
			case II_MOVE:
			{
				if (ii->out.type == IIOP_EDGE) findEdgeByIndex(ii->out.value)->intervalStart(iCounter);
				if (ii->arg.type == IIOP_EDGE) findEdgeByIndex(ii->arg.value)->intervalEnd(iCounter);
			}
			break;

			case II_COPY:
			{
				if (ii->out.type == IIOP_EDGE) findEdgeByIndex(ii->out.value)->intervalStart(iCounter);
			}
			break;

			default:
			{
				if (ii->isDyadic()) findEdgeByIndex(ii->arg.value)->intervalEnd(iCounter);
			}
		}

		iCounter++;
	}
}

//---------------------------------------------------------------------------------------------
// Currently allocator uses "first free" algorithm. It is known to be optimal for interval
// graphs. However it ignores boundary conditions (edges starting or ending at fixed
// registers). The edge list is already sorted by interval start, so it may be used directly.
// The code assumes that only one edge can start at given instruction.

bool Optimizer::allocateRegisters()
{
	Scheduler dataRegisters;
	Edge *edge = edges.first();
	int iCounter = 0;

	if (dataRegisters.start(32))
	{
		while (edge)
		{
			if (edge->getStart() == iCounter)
			{
				int dataRegister;
				
				dataRegister = dataRegisters.useFirstFor(edge->getEnd() - edge->getStart());

				if (dataRegister >= 0)
				{
					Operand tip(IIOP_DATAREG, dataRegister);
					edge->advanceTo(tip);
				}
				else
				{
					log.error("more than 32 registers used in %s(), compiler limit reached",
					 f->name);
					return FALSE;
				}
				
				edge = edge->next();
			}

			dataRegisters.tick();
			iCounter++;	
		}
		
		return TRUE;
	}
	else return FALSE;	
}

//---------------------------------------------------------------------------------------------

void Optimizer::applyRegistersToCode()
{
	Edge *e;
	
	for (InterInstruction *ii = f->code.first(); ii; ii = ii->next())
	{
		if (ii->arg.type == IIOP_EDGE)
		{
			e = findEdgeByIndex(ii->arg.value);
			ii->arg = e->getTip();
		}
		
		if (ii->out.type == IIOP_EDGE)
		{
			e = findEdgeByIndex(ii->out.value);
			ii->out = e->getTip();
		}
	}
}

//---------------------------------------------------------------------------------------------

void Optimizer::killRedundantMoves()
{
	for (InterInstruction *ii = f->code.first(); ii; ii = ii->next())
	{
		if ((ii->code == II_MOVE) || (ii->code == II_COPY))
		{
			if (ii->arg == ii->out) ii->remove();
		}	
	}
}
