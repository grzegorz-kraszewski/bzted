//---------------------------
// source scanner
//   * reads source file
//   * cuts file into tokens
//   * discards comments
//   * extracts strings
//---------------------------

#include <exec/types.h>

#include "token.h"


class Scanner
{
	char buf[64];
	int lineNum;
	int bufPos;
	int tokenCount;
	bool haveToken;
	bool comment;
	char stringMode;
	RpList<Token> *tokens;

	bool processChar(char c);
	bool addChar(char c);
	bool flush();

	public:

	Scanner(RpList<Token> *tokenList)
	{
		lineNum = 1;
		bufPos = 0;
		tokenCount = 0;
		haveToken = FALSE;
		comment = FALSE;
		stringMode = 0;
		tokens = tokenList;
	}

	bool scan(const char *filename);
};
