#ifndef _SAMPLE_EBS_TOOL_H_
#define _SAMPLE_EBS_TOOL_H_

//singleton class
#include <map>

#include "platform.hpp"
#include "tool.hpp"
#include "ebsDevice.hpp"
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

	void tool_EBSEventHandler(Event *event, Device * dev, int signum, siginfo_t *info, void *context);
	void tool_sessionInit();
	void tool_sessionExit();
	void tool_processInit();
        void tool_processExit();
        void tool_threadInit();
        void tool_threadExit();
	void tool_doPostProcess();

	void tool_eventHandler(Event *event, Device * dev, int signum, siginfo_t *info, void *context);
};

#endif
