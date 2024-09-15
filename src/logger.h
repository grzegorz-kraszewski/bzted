// Logging facility
//------------------

#define LOGLEVEL_DEBUG    1
#define LOGLEVEL_VERBOSE  2
#define LOGLEVEL_INFO     3
#define LOGLEVEL_WARNING  4
#define LOGLEVEL_ERROR    5

class Logger
{
	int level;
	const char *module;
	char faultBuffer[96];
	void log(int level, const char *msg, int *args);

	public:

	void error(const char *fmt, ...)
	{
		int *args = (int*)&fmt + 1;
		log(LOGLEVEL_ERROR, fmt, args);
	}

	void warning(const char *fmt, ...)
	{
		int *args = (int*)&fmt + 1;
		log(LOGLEVEL_WARNING, fmt, args);
	}

	void info(const char *fmt, ...)
	{
		int *args = (int*)&fmt + 1;
		log(LOGLEVEL_INFO, fmt, args);
	}

	void verbose(const char *fmt, ...)
	{
		int *args = (int*)&fmt + 1;
		log(LOGLEVEL_VERBOSE, fmt, args);
	}

	void debug(const char *fmt, ...)
	{
		int *args = (int*)&fmt + 1;
		log(LOGLEVEL_DEBUG, fmt, args);
	}

	void outOfMemory();
	void setLevel(int newlevel) { level = newlevel; }
	void setModule(const char *mod) { module = mod; }
	char* fault(int syserror, const char *prefix);
};

extern Logger log;