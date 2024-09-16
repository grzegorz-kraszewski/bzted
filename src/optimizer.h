//--------------------------
// immediate code optimizer
//--------------------------

#include "inter.h"

class Function;


class Edge : public RpNode<Edge>
{
	int eIndex;
	Operand tip;

	public:

	Edge(int index, Operand &start) { eIndex = index; tip = start; }
	void advanceTo(Operand &end) { tip = end; }
	bool tipIs(Operand &op) { return (tip == op); }
	void terminate() { tip.type = IIOP_NONE; }
	int index() { return eIndex; }
};


class Optimizer
{
	Function *f;
	RpList<Edge> edges;
	int edgeCount;

	Edge* addEdge(Operand &start);
	Edge* findEdgeByTip(Operand &op);
	void convertMoveToEdges(InterInstruction *ii);
	void convertCopyToEdges(InterInstruction *ii);
	void convertDyadicToEdges(InterInstruction *ii);
	void convertToEdges();

	public:

	Optimizer(Function *func) { f = func; edgeCount = 0; }
	void optimizeFunction();
};
