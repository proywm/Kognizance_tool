#ifndef _SAMPLE_PEBS_TOOL_H_
#define _SAMPLE_PEBS_TOOL_H_

//singleton class
#include <map>

#include "platform.hpp"
#include "tool.hpp"
#include "pebsDevice.hpp"
#include "bpDevice.hpp"
#include "context.hpp"

using namespace std;

class MyTool : public Platform, public Tool 
{
private:
	MyTool(){};
	MyTool(MyTool const&){};
	Tool& operator=(Tool const&){};
	static MyTool* t_pInstance;

public:
	static MyTool* Instance();

	void tool_PEBSEventHandler(Event *event, Device * dev, int signum, siginfo_t *info, void *context);
	void tool_BPEventHandler(Event *event, Device * dev, int signum, siginfo_t *info, void *context);
	void tool_sessionInit();
	void tool_sessionExit();
	void tool_processInit();
        void tool_processExit();
        void tool_threadInit();
        void tool_threadExit();
	void tool_doPostProcess();

	void tool_eventHandler(Event *event, Device * dev, int signum, siginfo_t *info, void *context);
	
	static thread_local int fd;
};

#endif
