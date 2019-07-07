#ifndef _TOOL_H_
#define _TOOL_H_

#include "context.hpp"

class Tool
{
private:
public:
	//Pure virtual function to be imeplemented by MyTool
//        virtual void ()=0;
	//Pure virtual function to be imeplemented by tool
        virtual void tool_sessionInit()=0;
        virtual void tool_sessionExit()=0;
        virtual void tool_eventHandler(Event *event, Device * dev, int signum, siginfo_t *info, void *context)=0;
        virtual void tool_processInit()=0;
        virtual void tool_processExit()=0;
        virtual void tool_threadInit()=0;
        virtual void tool_threadExit()=0;
	virtual void tool_doPostProcess()=0;
};

#endif
