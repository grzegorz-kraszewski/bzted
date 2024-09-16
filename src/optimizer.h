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

	Edge(Operand &start) { tip = start; }
	void advanceTo(Operand &end) { tip = end; }
	int index() { return eIndex; }
};


class Optimizer
{
	Function *f;
	RpList<Edge> edges;

	public:

	Optimizer(Function *func) { f = func; }
	void optimizeFunction();
};
