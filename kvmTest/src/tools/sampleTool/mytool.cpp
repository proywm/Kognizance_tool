#include <stddef.h>  // defines NULL
#include "mytool.hpp"

// Global static pointer used to ensure a single instance of the class.
MyTool* MyTool::t_pInstance = NULL;

/** This function is called to create an instance of the class. 
    Calling the constructor publicly is not allowed. The constructor 
    is private and is only called by this Instance function.
*/
  
MyTool* MyTool::Instance()
{
   if (!t_pInstance)   // Only allow one instance of class to be generated.
      t_pInstance = new MyTool;

   return t_pInstance;
}

void MyTool::tool_sessionInit()
{
	//do what you gotta do
	//register a PEBS device for this session/thread
	generate_session_key();
	int sessionId = get_session_key();
//	PEBSDevice *pebsdev = new PEBSDevice();
//	pebsdev->registerDevice(sessionId);
}

void MyTool::tool_sessionExit()
{
	int sessionId = get_session_key();
	//removing all the devices attached to this session
	remove_device_by_session(sessionId);
}

void MyTool::tool_processInit()
{
	tool_sessionInit();
}

void MyTool::tool_processExit()
{
	tool_sessionExit();
}

void MyTool::tool_threadInit()
{

}

void MyTool::tool_threadExit()
{

}


void MyTool::tool_eventHandler(Event *event, Device * dev, int signum, siginfo_t *info, void *context)
{
	std::cout << "Within tool handler: Doing tool specific handling\n";
        //do what you gotta do
}

void MyTool::tool_PEBSEventHandler(Event *event, Device * dev, int signum, siginfo_t *info, void *context)
{
        std::cout << "Within tool handler: Doing tool specific handling\n";
        //do what you gotta do
}

void MyTool::tool_BPEventHandler(Event *event, Device * dev, int signum, siginfo_t *info, void *context)
{
        std::cout << "Within tool handler: Doing tool specific handling\n";
//	OnWatchPoint(signum, info, context)
        //do what you gotta do
}

void MyTool::tool_doPostProcess(void)
{

}

