#ifndef _LOGGER_H
#define _LOGGER_H

#include <iostream>
#include <fstream> 
#include <string>
using namespace std;

extern std::ofstream *ofsInstance;

inline string getCurrentDateTime( string s ){
    time_t now = time(0);
    struct tm  tstruct;
    char  buf[80];
    tstruct = *localtime(&now);
    if(s=="now")
        strftime(buf, sizeof(buf), "%Y-%m-%d %X", &tstruct);
    else if(s=="date")
        strftime(buf, sizeof(buf), "%Y-%m-%d", &tstruct);
    return string(buf);
}

class MyLogger
{
private:
	MyLogger()
	{
		string filePath = "log_"+getCurrentDateTime("date")+".txt";
		ofsInstance = new ofstream(filePath.c_str(), std::ios_base::out | std::ios_base::app );
		cout << "file Location: " << filePath;
	};	
	static MyLogger* logInstance;

public:
	std::ofstream *ofsInstance;
	static MyLogger* Instance();
	void close();
};
#endif 
