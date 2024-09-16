//-----------------------------
// intermediate code optimizer
//-----------------------------

#include "optimizer.h"
#include "function.h"
#include "main.h"

#include <proto/dos.h>


//---------------------------------------------------------------------------------------------

void Optimizer::optimizeFunction()
{
	Printf("optimizing %s().\n", f->name);
	convertToEdges();
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
			case IIOP_IMMEDIATE:
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
			case II_COPY: convertCopyToEdges(ii); break;
			case II_ADDL:
			case II_SUBL: convertDyadicToEdges(ii); break; 
		}
	}
}