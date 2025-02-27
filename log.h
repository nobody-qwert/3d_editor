#ifndef LOG_H
#define LOG_H

#include <stdio.h>
#include "stdarg.h"

class LOG
{
	private:
		FILE* logfile;

	
	bool Init(void);
	bool Shutdown(void);


	public:


	bool Output(char* text, ...);

	LOG()
		{	Init();	}

	~LOG()
		{	Shutdown();	}
};

#endif
