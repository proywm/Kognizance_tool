#ifndef _BPDEVICE_H_
#define _BPDEVICE_H_

#include <iostream>
#include <linux/perf_event.h>
#include <linux/hw_breakpoint.h>

#include "device.hpp"
#include "myagent.hpp"


#if defined(PERF_EVENT_IOC_UPDATE_BREAKPOINT)
#define FAST_BP_IOC_FLAG (PERF_EVENT_IOC_UPDATE_BREAKPOINT)
#elif defined(PERF_EVENT_IOC_MODIFY_ATTRIBUTES)
#define FAST_BP_IOC_FLAG (PERF_EVENT_IOC_MODIFY_ATTRIBUTES)
#else
#endif



/*
*
* class BPDevice
* An abstract class
*
*/
class BPDevice : public Device
{

private:
	bool registerBPDevice();
	int length;
	uintptr_t address;
	int type;

public:
	BPDevice(int len, int bpType, uintptr_t addr)
	{
		length = len;
		address = addr;
		type = bpType;
		setDeviceType(BPDEVICE);
		devSignum = SIGRTMIN + 3;
//		std::cout << "BPDevice() type " << std::dec << bpType << " addr " << std::hex << addr << std::endl;
	}
	~BPDevice()
	{
		std::cout << "~BPDevice()" << std::endl;
	}
	int getType()
	{
		return type;
	}
	uintptr_t getAddr()
	{
		return address;
	}
	int getLength()
	{
		return length;
	}
	bool updateBPAddress(int len, int bpType, uintptr_t addr)
	{
		MyAgent *agent = MyAgent::Instance();
		
		if(agent->updateBPAddress(deviceFileId, len, bpType, addr))
		{
			isDisabled = 0;
			length = len;
        	        address = addr;
	                type = bpType;
			return true;
		}

		struct perf_event_attr bp_attr;
		memset(&bp_attr, 0, sizeof(struct perf_event_attr));

		registerThisDevice(&bp_attr);
		bp_attr.bp_len = len;
		bp_attr.bp_addr = addr;
		bp_attr.bp_type = bpType;
		printf("Came to update device \n");
		#if defined(FAST_BP_IOC_FLAG)
		printf("device id %d \n", deviceFileId);
		if(0 == (ioctl( deviceFileId, FAST_BP_IOC_FLAG, (unsigned long) (&bp_attr))))
		{
			pe_attr = bp_attr;
			isDisabled = 0;
			length = len;
        	        address = addr;
	                type = bpType;
			return true;
		}
		
		#endif
		return false;
	}
	//Pure virtual function to be imeplemented by tool
	int registerThisDevice(struct perf_event_attr *attr);
	void unRegisterThisDevice();
	void deviceOnEvent();
};


#endif
