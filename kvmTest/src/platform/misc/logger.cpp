#include "logger.hpp"

MyLogger* MyLogger::logInstance = NULL;

MyLogger* MyLogger::Instance()
{
	if (!logInstance)   // Only allow one instance of class to be generated.
	{
		logInstance = new MyLogger;
	}

	return logInstance;

}

void MyLogger::close()
{
	ofsInstance->close();	
}


