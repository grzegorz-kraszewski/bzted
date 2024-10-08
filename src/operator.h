//--------------------
// built-in operators
//--------------------

class Function;

// flags for operators and functions

struct Operator
{
	int numInputs;
	int numOutputs;
	bool (*generator)(Function *function);
};
