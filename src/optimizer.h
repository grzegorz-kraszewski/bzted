//--------------------------
// immediate code optimizer
//--------------------------

#include "inter.h"

class Function;


class Edge : public RpNode<Edge>
{
	int eIndex;
	Operand tip;
	int startInstruction;
	int endInstruction;

	public:

	Edge(int index, Operand &start) { eIndex = index; tip = start; }
	void advanceTo(Operand &end) { tip = end; }
	bool tipIs(Operand &op) { return (tip == op); }
	void terminate() { tip.type = IIOP_NONE; }
	int index() { return eIndex; }
	void intervalStart(int instr) { startInstruction = instr; }
	void intervalEnd(int instr) { endInstruction = instr - 1; }
	void print();
};


class Optimizer
{
	Function *f;
	RpList<Edge> edges;
	int edgeCount;

	Edge* addEdge(Operand &start);
	Edge* findEdgeByTip(Operand &op);
	Edge* findEdgeByIndex(int index);
	void convertMoveToEdges(InterInstruction *ii);
	void convertDmovToEdges(InterInstruction *ii);
	void convertCopyToEdges(InterInstruction *ii);
	void convertDyadicToEdges(InterInstruction *ii);
	void convertToEdges();
	void fuseImmediateOperands();
	void assignRegistersToArguments();
	void updateEdgesIntervals();
	void dumpEdges();

	public:

	Optimizer(Function *func) { f = func; edgeCount = 0; }
	void optimizeFunction();
};
