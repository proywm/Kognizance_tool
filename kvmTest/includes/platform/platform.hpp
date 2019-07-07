#ifndef _PLATFORM_H_
#define _PLATFORM_H_

#include <iostream>
#include <atomic>
#include <thread>
#include <map>
#include <mutex>
#include <unistd.h>
#include <sys/syscall.h>    /* For SYS_xxx definitions */

#include "monitor.h"
#include "device.hpp"
#include "fancyprinting.hpp"
#include "logger.hpp"

using namespace std;

static inline pid_t gettid() {
    return syscall(__NR_gettid);
}


void generic_dev_signal_handler(int signum, siginfo_t *info, void *context);

/*
*
* class Platform
* An abstract class
*
*/
class Platform
{
private:
	std::atomic<int> sKey;
	std::atomic<int> eKey;
	static thread_local int sessionKey;
	static thread_local volatile int inSignalHandler;
	std::map<pair<int, int>, Device*> deviceMap;	//to search by device id and threadId. This is required to find appropriated device handler from file/device id when an event occurs.
	std::multimap<int, int> deviceSessionMultiMap;	// to search by session key. This is required to turn on/off devices by session
	std::map<int, Event*> eventMap;	
	std::map<int, int> threadSessionMap;	

	std::mutex devicemap_mtx;
	std::mutex eventmap_mtx;
	std::mutex threadSessionmap_mtx;
public:
	Platform()
	{
		sKey = 0;
		eKey = 0;
	}
	void generate_session_key()
        {
//		if(sessionKey == -1)
//		sessionKey = sKey++;
		threadSessionmap_mtx.lock();
		sessionKey = sKey++;
		threadSessionMap[syscall(SYS_gettid)] = sessionKey;
		threadSessionmap_mtx.unlock();
        }
	void destroy_session_key(int threadId)
	{
		threadSessionmap_mtx.lock();
                threadSessionMap[threadId] = -1;
                threadSessionmap_mtx.unlock();
	}
	int get_session_key_from_threadId(int threadId)
	{
		int sessionId = -1;
		threadSessionmap_mtx.lock();
		sessionId = threadSessionMap[threadId];
		threadSessionmap_mtx.unlock();
		if(sessionId == -1)
			cout << ERRMSG_BEGIN << "Session key has been destroyed already for threadId " << std::dec << threadId << ERRMSG_END;
		return sessionId;
	}
	int get_session_key()
	{
		return sessionKey;
	}
	int generate_event_key()
        {
        	return eKey++;
        }
	
	bool isInSignalHandler()
	{
		return __sync_lock_test_and_set(&inSignalHandler, 1);
	}
	void exitingSignalHandler()
	{
		__sync_lock_release(&inSignalHandler);
	}
	void lockDevice()
	{
		devicemap_mtx.lock();
	}
	void unlockDevice()
	{
		devicemap_mtx.unlock();
	}
	void add_new_event(int eventId, Event *event)
	{
		eventmap_mtx.lock();
		eventMap[eventId] = event;
		eventmap_mtx.unlock();
	}
	// TODO remove events from map
	void remove_event(int eventId)
        {
                eventmap_mtx.lock();
		eventMap.erase(eventId);
                eventmap_mtx.unlock();
        }
	Event* get_event(int eventId)
        {
                Event* ret = nullptr;
		eventmap_mtx.lock();
		std::map<int, Event*>::iterator iter;
                iter = eventMap.find(eventId);
                if(iter != eventMap.end())
                        ret = iter->second;
		eventmap_mtx.unlock();
                return ret;
        }
	void add_new_device(int deviceFileId, Device *dev)
	{
		devicemap_mtx.lock();
		printf("Adding device id  %d - tid %d\n", deviceFileId, gettid());
		deviceMap[std::make_pair(gettid(), deviceFileId)] = dev;
		devicemap_mtx.unlock();
	}
	void add_new_device_to_session(int deviceFileId, int sessionId)
	{
		devicemap_mtx.lock();
		printf("Adding device id %d to session %d\n", deviceFileId, sessionId);
		deviceSessionMultiMap.insert(make_pair(sessionId, deviceFileId));
		devicemap_mtx.unlock();
	}
	/*
	* Case 1: remove_device Will take care of all the mapping while deleting itself.
	* Case 2: When remove_device device is called holding the deviceSessionMultiMap, one should call it with removeSessionMap=false.
	* This will refrain remove_device to update the session-device map. The called must explicitly remove this entry from the map.
	*
	* Lock(devicemap_mtx) before calling this function.
	*/
	void remove_device(int deviceFileId, bool removeSessionMap=true)
        {
//		printf("device id to remove %d \n", deviceFileId);
                std::map<pair<int, int>, Device*>::iterator iter;
                iter = deviceMap.find(std::make_pair(gettid(), deviceFileId));
                if(iter != deviceMap.end())
		{		
			if(removeSessionMap)
			{
				typedef multimap<int, int>::iterator mmapiter;
		                pair<mmapiter, mmapiter> sessionDevices =  deviceSessionMultiMap.equal_range(iter->second->getDeviceSessionId());
				for (mmapiter it = sessionDevices.first; it != sessionDevices.second; it++)
				{
					if(it->second == deviceFileId)
					{
						deviceSessionMultiMap.erase(it);
						break;
					}
				}
			}
			iter->second->end_device();
			iter->second->unRegisterDevice();
                        deviceMap.erase(std::make_pair(gettid(), deviceFileId));
		}
		else
			printf("device not found to remove  %d ------------------->\n", deviceFileId);
        }
	void remove_device_by_session(int sessionId)
	{
		devicemap_mtx.lock();
		typedef multimap<int, int>::iterator mmapiter;
                pair<mmapiter, mmapiter> sessionDevices =  deviceSessionMultiMap.equal_range(sessionId);
                // Iterate over the range
                for (mmapiter it = sessionDevices.first; it != sessionDevices.second; it++)
		{
			printf("device id %d to remove by sessionId %d\n", it->second, it->first);
			/* currently iterating deviceSessionMultiMap. So asking not to update this map with removeSessionMap=false.*/
			remove_device(it->second, false);
			
		}
//		printf("Done removing devices from sessionId %d \n", sessionId);
		deviceSessionMultiMap.erase(sessionId);
		devicemap_mtx.unlock();
	}
	Device* get_device(int threadId, int deviceFileId)
	{
		Device* ret = nullptr;
		devicemap_mtx.lock();
		std::map<pair<int, int>, Device*>::iterator iter;
                iter = deviceMap.find(std::make_pair(threadId, deviceFileId));
                if(iter != deviceMap.end())
		{
                        ret = iter->second;
		}
		else
		{
			std::cout << std::dec << "get_device failed: threadId " << threadId << " device id:  " << deviceFileId << std::endl;
		}
		devicemap_mtx.unlock();
		return ret;
	}
	
	void pause_all_devices()
	{
		devicemap_mtx.lock();
		std::map<pair<int, int>, Device*>::iterator iter;
		for (iter = deviceMap.begin(); iter != deviceMap.end(); iter++)
		{
			iter->second->disable_device();
//			std::cout << "Pausing device: " << std::dec << iter->first << "\n";
		}
		devicemap_mtx.unlock();
	}
	void pause_devices_by_session(int sessionId)
	{
		devicemap_mtx.lock(); 
                typedef multimap<int, int>::iterator mmapiter;
	//	std::cout << "Session Id: " << std::dec << sessionId << "\n";
                pair<mmapiter, mmapiter> sessionDevices =  deviceSessionMultiMap.equal_range(sessionId);
	//	std::cout << "Session devices: " << std::dec << sessionId << "\n";
                // Iterate over the range
                for (mmapiter it = sessionDevices.first; it != sessionDevices.second; it++)
                {
	//		std::cout << "this device: "<< std::dec << it->second << "\n";

			Device* dev = nullptr;//get_device(it->second);
			std::map<pair<int, int>, Device*>::iterator iter;
	                iter = deviceMap.find(std::make_pair(gettid(), it->second));
        	        if(iter != deviceMap.end())
                	{
                        	dev = iter->second;
                	}


			if((dev != nullptr) && !dev->isDisabled )
			{
	//			printf("Pausing device %d of sessionId %d\n", it->second, sessionId);
				dev->disable_device();
          //              	printf("Success on Pausing device %d of sessionId %d\n", it->second, sessionId);
			}
			else
			{
				if(dev == nullptr)
					printf("Could not Pause device %d of sessionId %d. Not valid: device is null\n", it->second, sessionId);
#if 0
				else
					printf("Could not Pause device %d of sessionId %d. Not valid: device is already disabled\n", it->second, sessionId);
#endif
			}
                 
                }
//		std::cout << "Done pausing Session Id: " << std::dec << sessionId << "\n";
                devicemap_mtx.unlock();
	}
	void resume_all_devices()
        {
		devicemap_mtx.lock();
                std::map<pair<int, int>, Device*>::iterator iter;
                for (iter = deviceMap.begin(); iter != deviceMap.end(); iter++)
                {
			if(!iter->second->isDisabled)
			{
				iter->second->enable_device();
//				std::cout << "Resuming device: " << std::dec << iter->first << "\n";
			}
                }
		devicemap_mtx.unlock();
        }
	void resume_devices_by_session(int sessionId)
	{
		devicemap_mtx.lock(); 
                typedef multimap<int, int>::iterator mmapiter;
                pair<mmapiter, mmapiter> sessionDevices =  deviceSessionMultiMap.equal_range(sessionId);
                // Iterate over the range
                for (mmapiter it = sessionDevices.first; it != sessionDevices.second; it++)
                {
			Device* dev = nullptr;//get_device(it->second);
                        std::map<pair<int, int>, Device*>::iterator iter;
                        iter = deviceMap.find(std::make_pair(gettid(),it->second));
                        if(iter != deviceMap.end())
                        {
                                dev = iter->second;
                        }


                        if((dev != nullptr) && !dev->isDisabled )
			{
//				printf("Going to Resume device %d of sessionId %d\n", it->second, sessionId);
				dev->enable_device();
//				printf("Resuming device %d of sessionId %d\n", it->second, sessionId);
                 	}
                }
                devicemap_mtx.unlock();
	}
	//returns number of possible devices
	int getMaxDevices(int deviceType);

	//friend function : generic_dev_signal_handler. It decides which device handler to call based on file id
	void generic_dev_signal_handler(int signum, siginfo_t *info, void *context);
};

#if 0
// process and thread initialization callbacks
void *monitor_init_process(int *argc, char **argv, void *data);
void monitor_fini_process(int how, void *data);
void *monitor_init_thread(int tid, void* data);
void monitor_fini_thread(void* init_thread_data);

//memory allocation callbacks
void monitor_pre_malloc(size_t size);
void monitor_post_malloc(size_t size, void *handle);
void monitor_pre_realloc(void *ptr, size_t size);
void monitor_post_realloc(void *ptr, size_t size, void *ptrTmp);
void monitor_pre_calloc(size_t nmemb, size_t size);
void monitor_post_calloc(size_t nmemb, size_t size, void *ptr);
void monitor_pre_memalign(size_t blocksize, size_t bytes);
void monitor_post_memalign(size_t blocksize, size_t bytes, void *ptr);
void monitor_pre_posix_memalign(void** memptr, size_t alignment, size_t size);
void monitor_post_posix_memalign(void** memptr, size_t alignment, size_t size, int ret);
void monitor_pre_free(void *handle);
#endif


#endif
