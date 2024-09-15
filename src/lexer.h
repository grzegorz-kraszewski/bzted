//----------------------------------
// lexer
//   * determines basic token types
//   * parses numeric literals
//----------------------------------

#include "token.h"

class Lexer
{
	RpList<Token> *tokens;
	
	void collectDefinitions();
	void updateIdentifiers();

	public:

	Lexer(RpList<Token> *tlist) { tokens = tlist; }
	void lex();
};
