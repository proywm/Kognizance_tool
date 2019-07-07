#include <stddef.h>  // defines NULL
#include <sys/syscall.h>    /* For SYS_xxx definitions */

#include "mytool.hpp"
#include "deadStore.hpp"
#include "redSpy.hpp"
#include "loadSpy.hpp"
#include "dupSpy.hpp"
#include "falsesharing.hpp"
#include "myagent.hpp"
#include "logger.hpp"

#include "bpDevice.hpp"

extern bool IsPCSane(void * contextPC, void *possiblePC);
extern void *  GetPatchedIP(void *  contextIP);
extern int GetFloorWPLength(int accessLen);

//extern bool get_mem_access_length_and_type_address(void * ip, uint32_t *accessLen, AccessType *accessType, FloatType * floatType, void * context, void** address);


// Global static pointer used to ensure a single instance of the class.
MyTool* MyTool::t_pInstance = NULL;

int MyTool::WPInitFlag	= 0;
#if defined WITCHTOOL_REDSPY
WitchLogicType MyTool::logicType = REDSPY;
#elif defined WITCHTOOL_DEADSPY
WitchLogicType MyTool::logicType = DEADSPY;
#elif defined WITCHTOOL_DUPSPY
WitchLogicType MyTool::logicType = DUPSPY;
#elif defined WITCHTOOL_FALSESHARE
WitchLogicType MyTool::logicType = FALSESHARE;
#else
WitchLogicType MyTool::logicType = LOADSPY;
#endif

#define INITONCE(x)  __sync_lock_test_and_set(&x, 1)

/** This function is called to create an instance of the class. 
    Calling the constructor publicly is not allowed. The constructor 
    is private and is only called by this Instance function.
*/
  
MyTool* MyTool::Instance()
{
   if (!t_pInstance)   // Only allow one instance of class to be generated.
   {
      t_pInstance = new MyTool;
   }
   
   return t_pInstance;
}

void MyTool::WPInitOnce()
{
    if(!INITONCE(WPInitFlag))
    {
	WP_Init();
    }
}

void MyTool::tool_sessionInit()
{
	//do what you gotta do
	//registering WatchPoint library
	//do it once
	
	WPInitOnce();
	//WP thread init
        WP_ThreadInit(MyTool::OnWatchpoint);

	//register a PEBS device for this session/thread
	generate_session_key();
	int sessionId = get_session_key();

	AccessType accessType = STORE;

	switch(logicType)
        {
		case LOADSPY:
#if defined WITCHTOOL_DUPSPY_LOAD
		case DUPSPY:
#endif
                {
                        accessType = LOAD;
                }
                break;
		case REDSPY:
        	case DEADSPY:
#if defined WITCHTOOL_DUPSPY_STORE
		case DUPSPY:
#endif
		case FALSESHARE:
		default:
                {
			accessType = STORE;
                }
         }


	PEBSDevice *pebsdev = new PEBSDevice(accessType);
	pebsdev->registerDevice(sessionId);
}

WP_TriggerAction_t MyTool::OnWatchpoint(WP_TriggerInfo_t *wpi){

    printf("The watched %lx has been trapped by IP %lx with accuracy %d\n", wpi->va, wpi->pc, wpi->pcPrecise);

    // TODO call DeadStore logic function
    
//    DeadStore *ds = DeadStore::Instance();
  //  ds->Witch_OnWP_Callback(event);

    return WP_DISABLE;
}

void MyTool::tool_sessionExit()
{
	WP_ThreadTerminate();
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

        //cout << TOOLMSG_BEGIN << "WICTH: Within PEBS handler: Doing tool specific handling: data Address: "<< std::hex<<event->h_data_addr << " and ip address: " << event->h_ip_addr << TOOLMSG_END << std::endl;
        //do what you gotta do

	if (event->h_data_addr != 0 && event->h_ip_addr != 0 && (event->h_ip_addr > 0xfffff00000000000) ){

		//Fix IP 
		if(!(event->isFixed || fixIP(event)))
		{
	                //std::cout << ERRMSG_BEGIN <<"WITCH: Failed to fix IP. Might impact on WitchTool" << ERRMSG_END << std::endl;
			return;
		}
		if(event->accessType == UNKNOWN)
			return;

#if 1
        if(event->h_ip_addr >= 0xffffffff810bb1f0 && event->h_ip_addr <= 0xffffffff811271a0)//0xffffffff810c1600) //ffffffff810c2110
                return;
#endif

#if 0
		if(event->h_ip_addr != 0xffffffff8129b79b)
                {
//                      pr_info("returning %llx \n", event->h_ip_addr);
                        return;
                }
                std::cout << "Will set BP .ip:" << std::hex << event->h_ip_addr << " and data addr: " << event->h_data_addr <<std::endl;
#endif

passAddressType:
#ifdef AVOID_PUSH

		if(event->inst == 0x000000000041)
		{
			delete event;
                        return;
		}
#endif

#ifdef AVOID_CALLQ 
		if(event->inst == 0x0000000000E8)
		{
//			std::cout << "Skipping as callq--------------------------------------------------------------------------------> \n";
			delete event;
			return;
		}
#endif

#ifdef AVOID_LOCK
                if(event->inst == 0x0000000000F0)
                {
//                      std::cout << "Skipping lock operation -------------------------------------------------------------------------------> \n";
                        delete event;
                        return;
                }
#endif

		switch(logicType)
		{
			case DEADSPY:
			{
				DeadStore *ds = DeadStore::Instance();
				//TODO enable following line
				if(!ds->Witch_OnSample_Callback(event))
					return;
			}
			break;
			case LOADSPY:
			{
				LoadSpy *ls = LoadSpy::Instance();
				if(!ls->Witch_OnSample_Callback(event))
					return;
			}
			break;
			case REDSPY:
                        {
                                RedSpy *rs = RedSpy::Instance();
                                //TODO enable following line
                                if(!rs->Witch_OnSample_Callback(event))
					return;
                        }
                        break;
			case DUPSPY:
                        {
                                DupSpy *ds = DupSpy::Instance();
                                //TODO enable following line
                                if(!ds->Witch_OnSample_Callback(event))
					return;
                        }
                        break;
			case FALSESHARE:
			{
				FalseSharing *fs = FalseSharing::Instance();
				if(!fs->Witch_OnSample_Callback(event))
                                        return;
			}
			break;
			default:
                        {
				
                        }

		}
//		printf(">>>>>>>>>>>>>>>>>> Witch Tool setting Breakpoint at getpid: %d getpthread_self: %lu tid:%lu \n",getpid(), pthread_self(), syscall(SYS_gettid));

		int watchLen = GetFloorWPLength(event->accessLength);
		int ret = 0;

		if(logicType == REDSPY)
			ret = WP_Subscribe((void *)event->h_data_addr, watchLen, event->accessLength, WP_WRITE, event);
		else if(logicType == DUPSPY)
			ret = WP_Subscribe((void *)event->h_ip_addr, sizeof(long), event->accessLength, WP_INST, event);
		else
			ret = WP_Subscribe((void *)event->h_data_addr, watchLen, event->accessLength, WP_RW, event);

		cout << SUCCESSMSG_BEGIN << "WITCH: Subscribed WP: data address: " << std::hex << event->data_addr << SUCCESSMSG_END  << std::endl;


#if 0
		if(ret == -1)
			cout << ERRMSG_BEGIN <<"WITCH: Error happen while Subscribing WP @" << std::hex << event->h_data_addr << ERRMSG_END << std::endl;
		else
			cout << SUCCESSMSG_BEGIN << "WITCH: Subscribed WP" << SUCCESSMSG_END << std::endl;
#endif
	}
}

void MyTool::tool_BPEventHandler(Event *event, Device * dev, int signum, siginfo_t *info, void *context)
{
#if 0
	if(!event)
		std::cout << ERRMSG_BEGIN << "WITCH: Within BP: EVENT IS NULL" << ERRMSG_END << std::endl;
        std::cout << TOOLMSG_BEGIN << "WICTH: Within BP handler: Doing tool specific handling. "<< std::hex<<event->h_data_addr << event->eventId << TOOLMSG_END << std::endl;

	//Fixing IP
	if(event->isFixed || fixIP(event))
	{
		 std::cout << SUCCESSMSG_BEGIN << "WITCH: IP was fixed" SUCCESSMSG_END << std::endl;
	}
	else
	{
		std::cout << ERRMSG_BEGIN <<"WITCH: Failed to fix IP. Might impact on WitchTool" << ERRMSG_END << std::endl;
	//	return ;
	}
#endif

	//TODO put Witch logic here
//	DeadStore *ds = DeadStore::Instance();
//        ds->Witch_OnWP_Callback(event, );

	OnWatchPoint(dev->getDeviceFildId(), context, event);
        //do what you gotta do
}

bool MyTool::fixIP(Event *event)
{
	void * contextIP = (void *) event->h_ip_addr;
	void * patchedIP;

	Device *dev = get_device(event->h_tId, event->deviceFileId);
	if(!dev) {
		std::cout<< "finding device failed \n";
		return false;
	}

	//if the event is precise event lvl 3, we don't need to get patchIP as its already fixed
	// additionally If we use instruction breakpoint, we do not need to fixIP
	if((get_device(event->h_tId, event->deviceFileId)->getDeviceType() == PEBSDEVICE) || ((dynamic_cast<BPDevice*> (get_device(event->h_tId, event->deviceFileId)))->getType()==HW_BREAKPOINT_X))
	{
		patchedIP = contextIP;
	}
	else
        	patchedIP = GetPatchedIP(contextIP);
        
	if(!IsPCSane(contextIP, patchedIP)) 
        	return false;
        else
		event->fixedIP = event->h_ip_addr = (uint64_t) patchedIP;

//	FloatType floatType = ELEM_TYPE_FLOAT16;
//      uint64_t pc = event->h_ip_addr;
        void * pc = (void *) event->h_ip_addr;
	
//       std::cout << "Generic Agent: parsing perf events ==============> " << std::endl;

	MyAgent *agent = MyAgent::Instance();
	void *addr = nullptr;
	if(get_device(event->h_tId, event->deviceFileId)->getDeviceType() == BPDEVICE)
	{
		void *addr = (void *)-1;
		if( (event->accessType == UNKNOWN) && (false == agent->get_mem_access_length_and_type_address(pc, (uint32_t*) &(event->accessLength), &(event->accessType), &(event->floatType), NULL, &addr)))
		{
			event->accessType = UNKNOWN;
//			std::cout << "Generic Agent: BP failed to get accessType and length ================> ip: "<< std::hex << event->h_ip_addr << std::endl;
			return false;
		}
		else
		{
//			std::cout << "Generic Agent BP: got accessType and length ++++++++++++++++> ip: "<< std::hex << event->h_ip_addr << " event->accessLength: " << event->accessLength << " accessType: " << event->accessType << " floatType : " << event->floatType << std::endl;
			event->h_data_addr = event->data_addr = (uintptr_t) addr;
//	                cout << "BPDEVICE DataAddr: " << event->data_addr;
		}
	}
	else
	{
        	if( (event->accessType == UNKNOWN) && (false == agent->get_mem_access_length_and_type_address(pc, (uint32_t*) &(event->accessLength), &(event->accessType), &(event->floatType), NULL, NULL)))
        	{
                	event->accessType = UNKNOWN;
//                	std::cout << "Generic Agent: PEBS failed to get accessType and length ================> ip: "<< std::hex << event->h_ip_addr << std::endl;
			return false;
        	}
        	else
        	{
//                	std::cout << "Generic Agent PEBS: got accessType and length ++++++++++++++++> ip: "<< std::hex << event->h_ip_addr << " event->accessLength: " << event->accessLength << " accessType: " << event->accessType << " floatType : " << event->floatType << std::endl;
        	}
	}
//	if(event->accessLength)
	event->valueAtEvent = (uint8_t *) malloc(sizeof(uint8_t *) * event->accessLength);
        int ret = agent->getValueAtAddress(event->data_addr, event->valueAtEvent, event->accessLength);
	if(ret < 0)
	{
//		std::cout << ERRMSG_BEGIN << " error while reading value at event address " << event->data_addr << std::endl << ERRMSG_END ;
		return false;
	}
	else
	{
//		std::cout << SUCCESSMSG_BEGIN<< " Successfully read value (" << event->valueAtEvent[0] << ") at event address " << event->data_addr << std::endl << SUCCESSMSG_END ;
	}

	//we would like to read the instruction itself
	uint64_t* tempInst = (uint64_t *) malloc(sizeof(uint64_t *) * 1);
	ret = agent->getValueAtAddress(event->fixedIP, (uint8_t*)tempInst, 4);
        if(ret < 0)
        {
//              std::cout << ERRMSG_BEGIN << " error while reading value at event address " << event->data_addr << std::endl << ERRMSG_END ;
                return false;
        }
        else
        {
		event->inst = (uint64_t)(((uint64_t)tempInst[0]) & 0x0000000000FF);

//		if(event->inst == 0x0000000000E8)
//		        std::cout << SUCCESSMSG_BEGIN<< " Successfully read value (" << (uint64_t)tempInst[0] << " and inst " << event->inst << ") at event address " << event->fixedIP << std::endl << SUCCESSMSG_END;
		free(tempInst);
        }

	//ip has been fixed.. reflect it in event's contextFrame

	if (!event->eventContext_agent.empty())
	{
		ContextFrame* cf = event->eventContext_agent.front();
		cf->binary_addr = event->h_ip_addr;
	}
	return true;
}

void MyTool::tool_doPostProcess(void)
{
	MyLogger *logger = MyLogger::Instance();

	*(logger->ofsInstance) << "Start writing results: " << std::endl;

	switch(logicType)
        {
        	case DEADSPY:
                {
                	DeadStore *ds = DeadStore::Instance();
                        //TODO enable following line
                        ds->printContextTree(0);
                }
                break;
                case LOADSPY:
                {
                	LoadSpy *ls = LoadSpy::Instance();
                        ls->printContextTree(0);
                }
                break;
                case REDSPY:
                {
                	RedSpy *rs = RedSpy::Instance();
                        //TODO enable following line
                        rs->printContextTree(0);
                }
                break;
		case DUPSPY:
                {
                        DupSpy *ds = DupSpy::Instance();
                        //TODO enable following line
                        ds->printContextTree(0);
                }
                break;
		case FALSESHARE:
                {
                	FalseSharing *fs = FalseSharing::Instance();
                        fs->printContextTree(0);
                }
                break;
                default:
                {

                }

        }

}

