#include "pebsDevice.hpp"
#include "myagent.hpp"

#define PEBS_SAMPLE_TYPE PERF_SAMPLE_IP | PERF_SAMPLE_TID | PERF_SAMPLE_ADDR | PERF_SAMPLE_ID | PERF_SAMPLE_CALLCHAIN | PERF_SAMPLE_CPU 
//| PERF_SAMPLE_INDEX

bool PEBSDevice::registerPEBSDevice()
{
	return false;
}
int PEBSDevice::registerThisDevice(struct perf_event_attr *attr)
{
	// Perf event settings
	attr->type                   = PERF_TYPE_RAW;
        attr->size                   = sizeof(struct perf_event_attr);
//        attr->sample_period          = 1000007;
        attr->sample_type            = PEBS_SAMPLE_TYPE;

	/* TODO  move from here. for kvm agent only*/
//        attr->exclude_guest             = 0;
	MyAgent *agent = MyAgent::Instance();
	if(agent->isAgentKVM())
	{
#ifdef PEBS_SAMPLING_PERIOD
		attr->sample_period             = PEBS_SAMPLING_PERIOD;
#else
		attr->sample_period          	= 10003;
#endif
	        attr->exclude_host              = 1;
		attr->exclude_user              = 0;
		attr->exclude_kernel            = 0;
		attr->exclude_hv	 	= 0;
		attr->disabled                  = 1; /* disabled */
	}
	else
	{
#ifdef PEBS_SAMPLING_PERIOD
                attr->sample_period             = PEBS_SAMPLING_PERIOD;
#else
		attr->sample_period          	= 1000003;
#endif
		attr->exclude_user             	= 0;
		attr->exclude_kernel            = 1;
		attr->exclude_guest             = 1;
		attr->exclude_callchain_user    = 0;
	        attr->disabled               	= 0; /* enabled */
	}

	switch(aType)
	{
		case LOAD:
		{
//			attr->config                 = 0x1cd;
#if defined PEBS_SAMPLING_L1_LOAD_MISS
			//attr->config                 = 0x5308D1; // L1 load miss
			attr->config		     = 0x8d1; // perf stat -e mem_load_uops_retired.l1_miss -vvv ls  // for broadwell
#elif defined PEBS_SAMPLING_LLC_LOAD_MISS
			attr->config                 = 0x5320D1; // LLC load miss
#else
			attr->config                 = 0x5381d0; //All Load
#endif
//			attr->config                 = 0x5308D1; // L1 load miss
//			attr->config                 = 0x5320D1; // LLC load miss
//                        attr->config1                = 0x3;
                        attr->precise_ip             = 3;
			break;
		}
		case STORE:
		default:
		{
        		attr->config                 = 0x5382d0;//0x2cd;
//			attr->config		     = 0x8d1;	//mem_load_uops_retired.l3_miss
//	        	attr->config1                = 0x0;
        		attr->precise_ip             = 3;
			break;
		}
	}

        attr->read_format            = PERF_FORMAT_GROUP | PERF_FORMAT_ID;
//        attr->task                   = 1;

	// fresh creation
//	return registerDevice(sessionId);
}

void PEBSDevice::unRegisterThisDevice()
{
//	unRegisterDevice();
}

void PEBSDevice::deviceOnEvent()
{
	//Event
}
