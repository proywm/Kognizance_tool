#include <iostream>

//#include "agent/pythonAgent/myagent.hpp"
#include "myagent.hpp"
#include "kvminterface.hpp"
#include "platform.hpp"
#include "pebsDevice.hpp"
#include "bpDevice.hpp"
#include "mytool.hpp"

/*
*
* Class MyAgent
* A class
*
*/

// Global static pointer used to ensure a single instance of the class.
MyAgent* MyAgent::a_pInstance = NULL;

extern bool get_mem_access_length_and_type_address__decoder(void * ip, uint32_t *accessLen, AccessType *accessType, FloatType * floatType, void * context, void** address);
extern void * get_previous_instruction__decoder(void *ins, void **pip, void ** excludeList, int numExcludes);

/** This function is called to create an instance of the class. 
    Calling the constructor publicly is not allowed. The constructor 
    is private and is only called by this Instance function.
*/

MyAgent* MyAgent::Instance()
{
   if (!a_pInstance)   // Only allow one instance of class to be generated.
      a_pInstance = new MyAgent;

   return a_pInstance;
}

void MyAgent::agent_getCallchain()
{
	std::cout << "NativeAgent: agent_getCallchain" << std::endl;
}

void MyAgent::agent_getAgentContext(Event *event)
{
	event->eventContext_agent = event->eventContext_host;
//      std::cout << "nativeAgent: agent_getCallchain" << std::endl;
}

bool MyAgent::isGuestSample(Event *event)
{
	for(auto iter = event->eventContext_host.begin(); iter != event->eventContext_host.end(); iter++)
	{
		cout << "function name:" << symbol->getFunctionName((*iter)->binary_addr) << "address: 0x"<<std::hex<< (*iter)->binary_addr <<std::endl;
	}
	return true;
}

void MyAgent::processEvent(Event *event)
{
	if (hyPerfDev != nullptr && hyPerfDev->dev )//&& isGuestSample(event))
	{
		VMEvent *evnt = (VMEvent*)malloc(sizeof(VMEvent));
		evnt->eventId = event->sample_index;//event->eventId;
		evnt->h_tId = event->h_tId;
		hyPerfDev->getEventState(hyPerfDev->dev, evnt);
//		std::cout << AGENT_MSG_BEGIN << ">>>>>>>>>>>>>>>>>>>>> event ID ="<< std::dec << event->sample_index << AGENT_MSG_END << std::endl;
	}
//	return getAgentContext__kvm(event);
}

void MyAgent::set_hyPerfDev(KVMInterface *dev)
{
	hyPerfDev = dev;
}

bool MyAgent::get_mem_access_length_and_type_address(void * ip, uint32_t *accessLen, AccessType *accessType, FloatType * floatType, void * context, void** address)
{
        return get_mem_access_length_and_type_address__decoder(ip, accessLen, accessType, floatType, context, address);
//	return false;
}

void * MyAgent::get_previous_instruction(void *ins, void **pip, void **excludeList, int numExcludes)
{
        return get_previous_instruction__decoder(ins, pip, excludeList, numExcludes);
//	return nullptr;
}


//to verify the settings
#define MAX_PEBS_EVENTS		8
struct debug_store {
	uint64_t	bts_buffer_base;
	uint64_t	bts_index;
	uint64_t	bts_absolute_maximum;
	uint64_t	bts_interrupt_threshold;
	uint64_t	pebs_buffer_base;
	uint64_t	pebs_index;
	uint64_t	pebs_absolute_maximum;
	uint64_t	pebs_interrupt_threshold;
	uint64_t	pebs_event_reset[MAX_PEBS_EVENTS];
};

void MyAgent::setupPEBSbuffer(void *vshmem_PEBS_ptr)
{
	uint64_t guest_buffer_base_virt_addr = (*(uint64_t*)vshmem_PEBS_ptr) + sizeof(uint64_t);
	std::cout << AGENT_MSG_BEGIN << "We have got guest virt address of DS Area  = "<< std::hex << guest_buffer_base_virt_addr << AGENT_MSG_END << std::endl;
	
	MyTool *tool = MyTool::Instance();
	tool->generate_session_key();
        int sessionId = tool->get_session_key();
        PEBSDevice *pebsDev = new PEBSDevice();
        pebsDev->registerDevice(sessionId);
	pebsDev->updateDSAddress(guest_buffer_base_virt_addr, ((uint64_t)(uint64_t*)vshmem_PEBS_ptr) + sizeof(uint64_t));
//	tool->remove_device_by_session(sessionId);
	tool->remove_device(pebsDev->getDeviceFildId());
	struct debug_store *ds = (struct debug_store *) (((uint64_t)(uint64_t*)vshmem_PEBS_ptr) + sizeof(uint64_t));
	std::cout << AGENT_MSG_BEGIN << "pebs_buffer_base is set to = "<< std::hex << ds->pebs_buffer_base << AGENT_MSG_END << std::endl;
}

bool MyAgent::updateBPAddress(int deviceId, int len, int bpType, uintptr_t addr)
{
	int breakno;
	if(hyPerfDev != nullptr && deviceVCPUMap[deviceId] != -1)
        {
#if 1
		for(breakno = 0; breakno < HBP_NUM; breakno++)
                {
                        if(VCPUBPdeviceSlotMap[deviceVCPUMap[deviceId]][breakno] == deviceId)
		                return hyPerfDev->updateBPAddress(deviceVCPUMap[deviceId], len, bpType, addr, breakno);
		}
#endif
//		return hyPerfDev->updateBPAddress(deviceVCPUMap[deviceId], len, bpType, addr, 0);
        }
        else
                std::cout << ERRMSG_BEGIN << " hyPerfDev is null"<< ERRMSG_END << std::endl;
	return false;
}

void MyAgent::registerDevice(Device* dev)
{
	int breakno;
	if(hyPerfDev != nullptr)
	{
		deviceVCPUMap[dev->getDeviceFildId()] = hyPerfDev->getVCPU();

		if(dev->getDeviceType() == BPDEVICE)
		{
			std::map<int, int[HBP_NUM]>::iterator it = VCPUBPdeviceSlotMap.find( deviceVCPUMap[dev->getDeviceFildId()] );
			if(it == VCPUBPdeviceSlotMap.end())
			{
//				std::cout << AGENT_MSG_BEGIN << "Registering device " << std::dec << dev->getDeviceFildId() << "for the firts time at agent \n" << AGENT_MSG_END;
				VCPUBPdeviceSlotMap[deviceVCPUMap[dev->getDeviceFildId()]][0] = dev->getDeviceFildId();
				for(breakno = 1; breakno < HBP_NUM; breakno++)
				{
					VCPUBPdeviceSlotMap[deviceVCPUMap[dev->getDeviceFildId()]][breakno] = -1;
				}
			}
			else
			{
				for(breakno = 0; breakno < HBP_NUM; breakno++)
				{
					if(VCPUBPdeviceSlotMap[deviceVCPUMap[dev->getDeviceFildId()]][breakno] == -1)
					{
						VCPUBPdeviceSlotMap[deviceVCPUMap[dev->getDeviceFildId()]][breakno] = dev->getDeviceFildId();
						std::cout << AGENT_MSG_BEGIN << "registerDevice: VCPU BP slot found " << breakno << std::endl << AGENT_MSG_END;
						break;
					}
				}
				if(breakno == HBP_NUM)
				{
					deviceVCPUMap[dev->getDeviceFildId()] = -1;
					std::cout << ERRMSG_BEGIN << " BP registration for KVM: failed to get an empty slot"<< ERRMSG_END << std::endl;
					return;
				}
			}
			MyTool *tool = MyTool::Instance();
	                BPDevice* bpdev = dynamic_cast<BPDevice*> (dev);
			if(updateBPAddress(bpdev->getDeviceFildId(), bpdev->getLength(), bpdev->getType(), bpdev->getAddr()))
	                {
				std::cout << AGENT_MSG_BEGIN << "registerDevice: VCPU BP slot updated \n"<< AGENT_MSG_END;
				
        	                return;
                	}
		}
	}
	else
		std::cout << ERRMSG_BEGIN << " hyPerfDev is null"<< ERRMSG_END << std::endl;
}

void MyAgent::unRegisterDevice(Device* dev)
{
	int breakno;
#if 1
	if(dev->getDeviceType() == BPDEVICE)
	{
		for(breakno = 0; breakno < HBP_NUM; breakno++)
		{
			if(VCPUBPdeviceSlotMap[deviceVCPUMap[dev->getDeviceFildId()]][breakno] == dev->getDeviceFildId())
			{
				VCPUBPdeviceSlotMap[deviceVCPUMap[dev->getDeviceFildId()]][breakno] = -1;
			}
		}
	}
#endif
	deviceVCPUMap[dev->getDeviceFildId()] = -1;
}

bool MyAgent::pauseDevice(Device* dev)
{
	if(hyPerfDev != nullptr && deviceVCPUMap[dev->getDeviceFildId()] != -1)
        {

		int breakno;
                BPDevice* bpdev = dynamic_cast<BPDevice*> (dev);
                for(breakno = 0; breakno < HBP_NUM; breakno++)
                {
                        if(VCPUBPdeviceSlotMap[deviceVCPUMap[dev->getDeviceFildId()]][breakno] == dev->getDeviceFildId())
                        {
          //                      std::cout << AGENT_MSG_BEGIN << "pauseDevice: dev: " << std::dec << bpdev->getDeviceFildId() << " breakno "<< breakno << std::endl << AGENT_MSG_END;
                                return hyPerfDev->pauseBP(deviceVCPUMap[bpdev->getDeviceFildId()], bpdev->getLength(), bpdev->getType(), breakno);
                        }
                }
		std::cout << ERRMSG_BEGIN << "pauseDevice: failed " << std::dec << dev->getDeviceFildId() << std::endl << ERRMSG_END;
        }
        else
                std::cout << ERRMSG_BEGIN << " hyPerfDev is null"<< ERRMSG_END << std::endl;
        return false;
}

bool MyAgent::resumeDevice(Device* dev)
{
	if(hyPerfDev != nullptr && deviceVCPUMap[dev->getDeviceFildId()] != -1)
        {
		int breakno;

                BPDevice* bpdev = dynamic_cast<BPDevice*> (dev);
		for(breakno = 0; breakno < HBP_NUM; breakno++)
                {
                        if(VCPUBPdeviceSlotMap[deviceVCPUMap[bpdev->getDeviceFildId()]][breakno] == bpdev->getDeviceFildId())
			{
	//			std::cout << AGENT_MSG_BEGIN << "resumeDevice: dev: " << std::dec << bpdev->getDeviceFildId() << " breakno "<< breakno << std::endl << AGENT_MSG_END;
                                return hyPerfDev->resumeBP(deviceVCPUMap[dev->getDeviceFildId()], bpdev->getLength(), bpdev->getType(), breakno);
			}
                }
		std::cout << ERRMSG_BEGIN << "resumeDevice: failed " << std::dec << dev->getDeviceFildId() << std::endl << ERRMSG_END;

        }
        else
                std::cout << ERRMSG_BEGIN << " hyPerfDev is null"<< ERRMSG_END << std::endl;
        return false;
}

//int MyAgent::getGuestPhysicalMemory(uint64_t gpa, uint8_t *hostAddr, int len)
int MyAgent::getValueAtAddress(uint64_t gpa, uint8_t *hostAddr, int len)
{
	if(hyPerfDev != nullptr)
		return hyPerfDev->getGuestPhysicalMemory(gpa, hostAddr, len);
	return -1;
}

int MyAgent::kvm_get_regs(struct kvm_regs* regs)
{
        if(hyPerfDev != nullptr)
                return hyPerfDev->kvm_get_regs(regs);
        return -1;
}

