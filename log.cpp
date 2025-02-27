#include "log.h"


bool LOG::
	Init(void)	
{
	//Clear the log contents
	if((logfile=fopen("Log.txt", "wb"))==NULL)
		return false;

	//Close the file, and return a success!
	fclose(logfile);
	return true;
}


bool LOG::
	Shutdown(void)
{
	return true;
}


bool LOG::
	Output(char* text, ...)
{
	va_list arg_list;

	//Initialize variable argument list
	va_start(arg_list, text);

	//Open the log file for append
	if((logfile = fopen("Log.txt", "a+"))==NULL)
		return false;

	//Write the text and a newline
	vfprintf(logfile, text, arg_list);
	putc('\n', logfile);

	//Close the file
	fclose(logfile);
	va_end(arg_list);

	return true;
}
