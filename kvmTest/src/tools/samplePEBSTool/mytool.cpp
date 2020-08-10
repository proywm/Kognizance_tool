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
	PEBSDevice *pebsdev = new PEBSDevice();
//	pebsDev->updateDSAddress(0, 0);
	pebsdev->registerDevice(sessionId);
}

void MyTool::tool_sessionExit()
{
	int sessionId = get_session_key();
	//removing all the devices attached to this session
	remove_device_by_session(sessionId);
	destroy_session_key(sessionId);
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
	tool_sessionInit();
}

void MyTool::tool_threadExit()
{
	tool_sessionExit();
}


void MyTool::tool_eventHandler(Event *event, Device * dev, int signum, siginfo_t *info, void *context)
{
//	std::cout << "Within tool handler: Doing tool specific handling\n";
        //do what you gotta do
	switch(dev->getDeviceType())
        {
                case PEBSDEVICE :
                {
                        tool_PEBSEventHandler(event, dev, signum, info, context);
                        break;
                }
                case BPDEVICE :
                {
                        tool_BPEventHandler(event, dev, signum, info, context);
                        break;
                }
        }

}

void MyTool::tool_PEBSEventHandler(Event *event, Device * dev, int signum, siginfo_t *info, void *context)
{
	if (!event || (event && (event->data_addr == 0 || event->ip_addr == 0)))
		return;


       // std::cout << TOOLMSG_BEGIN << "Within tool handler: Doing tool specific handling: eventId " << event->eventId << "data address" << event->data_addr <<"\n"  << TOOLMSG_END;
//        if(event->ip_addr < 0x400d910)
	std::cout << "callstack ";
//	std::cout <<  event->eventId << std::hex << " "<< event->data_addr << " "<< event->ip_addr << std::dec<< std::endl << std::endl; 
	
	for(auto iter = event->eventContext_host.rbegin(); iter != event->eventContext_host.rend(); iter++)
		std::cout  << std::hex<< (*iter)->binary_addr << " " << std::dec;
	std::cout << std::endl;
//	PEBSDevice *pebsDev = (PEBSDevice*)dev;
//	pebsDev->updateDSAddress(0, 0);
        //do what you gotta do
}

void MyTool::tool_BPEventHandler(Event *event, Device * dev, int signum, siginfo_t *info, void *context)
{
        std::cout << TOOLMSG_BEGIN << "Within tool handler: Doing tool specific handling\n" << TOOLMSG_END;
//	OnWatchPoint(signum, info, context)
        //do what you gotta do
}

void MyTool::tool_doPostProcess(void)
{

}

