#ifndef _WITCHTOOL_H_
#define _WITCHTOOL_H_

//singleton class
#include <map>
#include <vector>
#include <mutex>

#include "platform.hpp"
#include "tool.hpp"
#include "pebsDevice.hpp"
#include "context.hpp"

#include "watchpoint.h"
#include "watchpoint_util.h"
#include "watchpoint_mmap.h"

using namespace std;
enum WitchLogicType { DEADSPY, REDSPY, LOADSPY, DUPSPY, FALSESHARE };

class MyTool : public Platform, public Tool 
{
private:
	MyTool(){WPInitOnce();};
	MyTool(MyTool const&){};
	Tool& operator=(Tool const&){};
	void WPInitOnce();

	static MyTool* t_pInstance;
	static int WPInitFlag;

	std::mutex tool_mtx;
	
public:
	static WitchLogicType logicType;
	static MyTool* Instance();
	static WP_TriggerAction_t OnWatchpoint(WP_TriggerInfo_t *wpi);
	bool fixIP(Event *event);

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

	void lockTool()
        {
                tool_mtx.lock();
        }
        void unlockTool()
        {
                tool_mtx.unlock();
        }

};

#endif
