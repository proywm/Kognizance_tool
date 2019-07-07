#include <stddef.h>  // defines NULL
#include "mytool.hpp"

thread_local int MyTool::fd = -1;
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
	std::cout << "Within tool handler: Doing tool specific handling\n";
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
//        std::cout << TOOLMSG_BEGIN << "Within PEBS handler: Doing tool specific handling : data address "<< std::hex << (uintptr_t)event->h_data_addr<< " ip: " << event->ip_addr << std::endl << TOOLMSG_END;
	if((uintptr_t)event->h_data_addr == 0)
		return;
        //do what you gotta do
	/****************************************************************************************************************************************************/
                int bp_len;
                int bp_type;
		void *va = (void *)event->h_data_addr;
                uintptr_t bp_addr = (uintptr_t)va;
                int watch_length = HW_BREAKPOINT_LEN_1;
                switch (watch_length) {
                        case 1: bp_len = HW_BREAKPOINT_LEN_1; break;
                        case 2: bp_len = HW_BREAKPOINT_LEN_2; break;
                        case 4: bp_len = HW_BREAKPOINT_LEN_4; break;
                        case 8: bp_len = HW_BREAKPOINT_LEN_8; break;
                        default:
                                printf("Unsupported .bp_len %d: %s\n", watch_length,strerror(errno));
                                exit(0);
                }
                int bpType = HW_BREAKPOINT_W | HW_BREAKPOINT_R;

                if(fd==-1)
                {
                        int sessionId = get_session_key();
                        BPDevice *bpdev = new BPDevice( bp_len, bpType, (uintptr_t)bp_addr);
			std::cout << TOOLMSG_BEGIN << "Going to register BP device. session: "<< sessionId << " bp_len "<< bp_len <<  " bp_type: " << HW_BREAKPOINT_W << " pebs ip: " << event->ip_addr << "data addr: " << event->data_addr << std::endl << TOOLMSG_END;
                        if(bpdev->registerDevice(sessionId) != 0)
			{
				std::cout << ERRMSG_BEGIN << "failed to register BP device \n" << ERRMSG_END ;
				return;
			}
                        fd = bpdev->getDeviceFildId();
                }
                else
                {
                        MyTool *tool = MyTool::Instance();
                        BPDevice* bpdev = dynamic_cast<BPDevice*> (tool->get_device(gettid(), fd));
			if(bpdev == nullptr)
			{
				fd = -1;
				return ;
			}
		//	std::cout << TOOLMSG_BEGIN << "Going to update BP device bp_len "<< std::dec << bp_len <<  " bp_type: " << HW_BREAKPOINT_W << "and fd "<< fd << std::endl << TOOLMSG_END;
			std::cout << TOOLMSG_BEGIN << "Going to register BP device. " << " bp_len "<< bp_len <<  " bp_type: " << HW_BREAKPOINT_W << " pebs ip: " << event->ip_addr << " data addr: " << event->data_addr << std::endl << TOOLMSG_END;
                        if(!bpdev->updateBPAddress(bp_len, bpType, (uintptr_t)bp_addr))
                        {
                                std::cout << ERRMSG_BEGIN << "failed to update breakpoint >>>>>>>>>>>>>>>>>>>>>>>>>>>>" << std::hex << bp_addr << std::dec << std::endl << ERRMSG_END;
                                return;
                        }

                }

	/****************************************************************************************************************************************************/
}

void MyTool::tool_BPEventHandler(Event *event, Device * dev, int signum, siginfo_t *info, void *context)
{
  //      std::cout << TOOLMSG_BEGIN << "Within BP handler: Doing tool specific handling" << " ip: " << std::hex << event->ip_addr <<  TOOLMSG_END << std::endl;
//	if(event->ip_addr == 0x400864)
	std::cout << TOOLMSG_BEGIN << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>BP handler " << std::hex << event->ip_addr << " data addr: " << event->data_addr << TOOLMSG_END << std::endl;
	MyTool *tool = MyTool::Instance();
   	dev->end_device();
//	OnWatchPoint(signum, info, context)
        //do what you gotta do
}

void MyTool::tool_doPostProcess(void)
{

}

