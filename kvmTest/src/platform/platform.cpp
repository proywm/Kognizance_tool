#include <atomic>
#include <sys/syscall.h>    /* For SYS_xxx definitions */

#include "platform.hpp"
#include "mytool.hpp"
#include "myagent.hpp"
#include "bpDevice.hpp"
#include "ebsDevice.hpp"
#include "pebsDevice.hpp"

thread_local int Platform::sessionKey = -1;
thread_local volatile int Platform::inSignalHandler = 0;

thread_local volatile int signalUnsafe = 0;

int mallocCount = 0;

#define ENTERING_SIGNAL_UNSAFE  (__sync_lock_test_and_set(&signalUnsafe, 1));

#define EXITING_SIGNAL_UNSAFE __sync_lock_release(&signalUnsafe);


void generic_dev_signal_handler(int signum, siginfo_t *info, void *context)
{
	int fd = info->si_fd;
	MyTool *tool = MyTool::Instance();
	
	/*
        *------------------------------
        * Pausing all the devices Only If Agent is not pausing
        *------------------------------
        */
	MyAgent *agent = MyAgent::Instance();
        if(!agent->requestWillBeNotified)
		tool->pause_all_devices();

	/*
	*---------------------------------------------------------------------------------------
	* Avoiding signal handling if signal was generated while running signal unsafe function
	*---------------------------------------------------------------------------------------
	*/
	if(__sync_lock_test_and_set(&signalUnsafe, 1))
	{
		if(!agent->requestWillBeNotified)
			tool->resume_all_devices();
		return;
	}
	__sync_lock_release(&signalUnsafe);
	

	/*
        *--------------------------------------------------------------------------------------------------------
        * Avoiding signal handling if already handling a signal. avoiding Re-entrance. Not sure whether necessary
        *--------------------------------------------------------------------------------------------------------
        */
	if(tool->isInSignalHandler())
	{
		if(!agent->requestWillBeNotified)
			tool->resume_all_devices();
		return;
	}
	
	/*
        *----------------------------------------------
        * Calling appropriate handler for the device
        *----------------------------------------------
        */
	Device *dev = tool->get_device(gettid(), fd);
	if(dev != nullptr)
		dev->dev_signal_handler(signum, info, context);
	
	tool->exitingSignalHandler();

	/*
        *---------------------------------------------------------
        * Resuming all the devices Only If agent is not notifying
        *---------------------------------------------------------
        */
	//MyAgent *agent = MyAgent::Instance();
	if(!agent->requestWillBeNotified)
		tool->resume_all_devices();
	
}

int Platform::getMaxDevices(int deviceType)
{
	int count = 0;
	generate_session_key();
	int sessionId = get_session_key();
	
	volatile int dummyWP[100];
	Device *dev = nullptr;
	while(count < MAX_POSSIBLE_DEVICES)
	{
		switch(deviceType)
		{
			case PEBSDEVICE:
			{
	        		dev = new PEBSDevice();
				break;
			}
			case BPDEVICE:
			{
				dev = new BPDevice( HW_BREAKPOINT_LEN_1, HW_BREAKPOINT_W, (uintptr_t)&dummyWP[count]);
				break;
			}
			case EBSDEVICE:
                	{
                        	dev = new EBSDevice();
				break;
                	}
			default:
				return count;
		}
	   	if(dev == nullptr)
			return 0;
	        int ret = dev->registerDevice(sessionId);
		if(ret == 0)
		{
			count++;
		}
		else
			break;
	}
	remove_device_by_session(sessionId);
	std::cout << "Max devices possible: " << count << std::endl;
	return count;
}

#if 1
//memory allocation callbacks
void monitor_pre_malloc(size_t size)
{
        ENTERING_SIGNAL_UNSAFE
        //std::cout << "within pre_malloc" << std::endl;
        mallocCount++;
}
void monitor_post_malloc(size_t size, void *handle)
{
        EXITING_SIGNAL_UNSAFE
}
void monitor_pre_realloc(void *ptr, size_t size)
{
        ENTERING_SIGNAL_UNSAFE
}
void monitor_post_realloc(void *ptr, size_t size, void *ptrTmp)
{
        EXITING_SIGNAL_UNSAFE
}
void monitor_pre_calloc(size_t nmemb, size_t size)
{
        ENTERING_SIGNAL_UNSAFE
//      std::cout << "within pre_calloc" << std::endl;
}
void monitor_post_calloc(size_t nmemb, size_t size, void *ptr)
{
        EXITING_SIGNAL_UNSAFE
}
void monitor_pre_memalign(size_t blocksize, size_t bytes)
{
        ENTERING_SIGNAL_UNSAFE
}
void monitor_post_memalign(size_t blocksize, size_t bytes, void *ptr)
{
        EXITING_SIGNAL_UNSAFE
}
void monitor_pre_posix_memalign(void** memptr, size_t alignment, size_t size)
{
        ENTERING_SIGNAL_UNSAFE
}
void monitor_post_posix_memalign(void** memptr, size_t alignment, size_t size, int ret)
{
        EXITING_SIGNAL_UNSAFE
}
void monitor_pre_free(void *handle)
{
        ENTERING_SIGNAL_UNSAFE
}
void monitor_post_free(void *handle)
{
        EXITING_SIGNAL_UNSAFE
}
#endif

