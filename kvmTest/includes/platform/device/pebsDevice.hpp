#ifndef _PEBSDEVICE_H_
#define _PEBSDEVICE_H_

#include <iostream>

#include "device.hpp"
#include "context.hpp"
#include "fancyprinting.hpp"

#if defined(PERF_EVENT_IOC_UPDATE_DS_AREA)
#define PEBS_UPDATE_DS_AREA (PERF_EVENT_IOC_UPDATE_DS_AREA)
#else
#endif

#if 0
struct ds_g2h_map
{
	uint64_t guest_buffer_base_virt_addr;
	uint64_t host_buffer_base_virt_addr;
};
#endif 

/*
*
* class PEBSDevice
* An abstract class
*
*/
class PEBSDevice : public Device
{

private:
	bool registerPEBSDevice();

	AccessType aType;
public:
	PEBSDevice()
	{
		setDeviceType(PEBSDEVICE);
		devSignum = SIGRTMIN + 4;
		aType = STORE;
		std::cout << "PEBSDevice()" << std::endl;
	}
	PEBSDevice(AccessType type)
	{
		setDeviceType(PEBSDEVICE);
                devSignum = SIGRTMIN + 4;
		aType = type;
		std::cout << "PEBSDevice()" << std::endl;
	}
	~PEBSDevice()
	{
		std::cout << "~PEBSDevice()" << std::endl;
	}
#if 0
	void setDSRegions(uint64_t guest_buffer_base_virt_addr, uint64_t host_buffer_base_virt_addr, uint64_t cpus)
	{
		int c = 0;
		for(; c < cpus; c++)
		{
			uint64_t guest_base_this_cpu = (c * (sizeof(struct debug_store)+BTS_BUFFER_SIZE+ x86_pmu.pebs_buffer_size + PEBS_FIXUP_SIZE)) + ds_area->guest_buffer_base_virt_addr;
	                uint64_t host_base_this_cpu = (c * (sizeof(struct debug_store)+BTS_BUFFER_SIZE+ x86_pmu.pebs_buffer_size + PEBS_FIXUP_SIZE)) + ds_area->host_buffer_base_virt_addr;

			struct debug_store *ds = host_base_this_cpu;
			//set bts base
	                uint64_t guest_BTS_base_this_cpu = guest_base_this_cpu + sizeof(struct debug_store);
			
			int max = BTS_BUFFER_SIZE / BTS_RECORD_SIZE;
        	        int thresh = max / 16;

                	ds->bts_buffer_base = guest_BTS_base_this_cpu;
                	ds->bts_index = ds->bts_buffer_base;
                	ds->bts_absolute_maximum = ds->bts_buffer_base +
                                                max * BTS_RECORD_SIZE;
                	ds->bts_interrupt_threshold = ds->bts_absolute_maximum -
                                                thresh * BTS_RECORD_SIZE;

			//set pebs base

	                uint64_t guest_PEBS_base_this_cpu = guest_BTS_base_this_cpu + BTS_BUFFER_SIZE;
			max = x86_pmu.pebs_buffer_size / x86_pmu.pebs_record_size;

	                ds->pebs_buffer_base = guest_PEBS_base_this_cpu;
        	        ds->pebs_index = ds->pebs_buffer_base;
                	ds->pebs_absolute_maximum = ds->pebs_buffer_base +
                        max * x86_pmu.pebs_record_size;
		}
	}
#endif
	bool updateDSAddress(uint64_t guest_buffer_base_virt_addr, uint64_t host_buffer_base_virt_addr)
        {
                printf("Came to update Debug store save area of PEBS device \n");
                #if defined(PEBS_UPDATE_DS_AREA)
                printf("device id %d \n", deviceFileId);
		struct ds_g2h_map ds_area;
		ds_area.guest_buffer_base_virt_addr = guest_buffer_base_virt_addr;
		ds_area.host_buffer_base_virt_addr = host_buffer_base_virt_addr;

		//before sending them to kernel, set them first.

		//get the number of possible cpus in host.
		unsigned long cpus = 0;
		if(0 == (ioctl( deviceFileId, PERF_EVENT_IOC_POSSIBLE_CPUS, (unsigned long) (&cpus))))
		{
			std::cout << PLATFORM_MSG_BEGIN << "Got possible cpus " << cpus << PLATFORM_MSG_END << std::endl;
		}
		else
		{
			std::cout << ERRMSG_BEGIN << "UNEXPECTED Behavior: Could not get possible cpus " << ERRMSG_END << std::endl;
			return false;
		}
//		setDSRegions(guest_buffer_base_virt_addr, host_buffer_base_virt_addr, cpus);
		
                if(0 == (ioctl( deviceFileId, PEBS_UPDATE_DS_AREA, (unsigned long) (&ds_area))))
                {
			std::cout << PLATFORM_MSG_BEGIN	<< "DS AREA has been updated successfully " << PLATFORM_MSG_END << std::endl;
                        return true;
                }
		else
		{
			 std::cout << PLATFORM_MSG_BEGIN << "DS AREA was not updated" << PLATFORM_MSG_END << std::endl;
		}
		#else
		std::cout << ERRMSG_BEGIN << "UNEXPECTED Behavior: No IOCTL found to update DS Area " << ERRMSG_END << std::endl;
                #endif
                return false;
        }


	//Pure virtual function to be imeplemented by tool
	int registerThisDevice(struct perf_event_attr *attr);
	void unRegisterThisDevice();
	void deviceOnEvent();
};


#endif
