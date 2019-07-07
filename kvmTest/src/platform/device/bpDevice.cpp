#include "bpDevice.hpp"
#include "myagent.hpp"

#define BP_SAMPLE_TYPE PERF_SAMPLE_IP | PERF_SAMPLE_TID | PERF_SAMPLE_ADDR | PERF_SAMPLE_CALLCHAIN | PERF_SAMPLE_CPU 
//| PERF_SAMPLE_INDEX

bool BPDevice::registerBPDevice()
{
	return false;
}
int dummy[128];
int BPDevice::registerThisDevice(struct perf_event_attr *attr)
{
	// Perf event settings
	attr->type			= PERF_TYPE_BREAKPOINT;
        attr->size			= sizeof(struct perf_event_attr);
	MyAgent *agent = MyAgent::Instance();
        if(agent->isAgentKVM())
	{
		attr->bp_type			= type;
		attr->bp_len			= length;
		attr->bp_addr 			= dummy[0];

		attr->exclude_user           	= 0;
        	attr->exclude_kernel         	= 1;
		attr->exclude_hv                = 1;

	/* TODO  move from here. for kvm agent only*/
//	attr->exclude_guest		= 0;
//	attr->exclude_host		= 1;
	}
	else
	{
		attr->bp_type                   = type;
	        attr->bp_len                    = length;
        	attr->bp_addr                   = (uintptr_t) address;

	        attr->exclude_user              = 0;
        	attr->exclude_kernel            = 1;
	        attr->exclude_hv                = 1;
	}

        attr->sample_period		= 1;
        attr->sample_type             	= BP_SAMPLE_TYPE;
        attr->disabled                	= 0; /* enabled */
	attr->precise_ip             	= 2;

	// fresh creation
//	return registerDevice(sessionId);
}

void BPDevice::unRegisterThisDevice()
{
//	unRegisterDevice();
}

void BPDevice::deviceOnEvent()
{
	//Event
}
