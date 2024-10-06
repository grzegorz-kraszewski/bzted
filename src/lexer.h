//----------------------------------
// lexer
//   * determines basic token types
//   * parses numeric literals
//----------------------------------

#include "token.h"

class Lexer
{
	RpList<Token> &tokens;
	
	bool collectDefinitions();
	bool updateIdentifiers();

	public:

	Lexer(RpList<Token> &tlist) : tokens(tlist) {}
	bool lex();
};
