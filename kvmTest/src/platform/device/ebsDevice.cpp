#include "ebsDevice.hpp"

#define EBS_SAMPLE_TYPE PERF_SAMPLE_IP | PERF_SAMPLE_TID | PERF_SAMPLE_TIME | PERF_SAMPLE_CPU | PERF_SAMPLE_PERIOD

bool EBSDevice::registerEBSDevice()
{
	return false;
}
int EBSDevice::registerThisDevice(struct perf_event_attr *attr)
{
	// Perf event settings
	attr->type                   = PERF_TYPE_HW_CACHE;
        attr->size                   = sizeof(struct perf_event_attr);
        attr->sample_period          = 4000;
        attr->sample_type            = EBS_SAMPLE_TYPE;
//        attr->exclude_user           = 1;
//	attr->exclude_kernel         = 1;
//	attr->exclude_hv             = 1;


	/* TODO  move from here. for kvm agent only*/
//        attr->exclude_guest             = 0;
        attr->exclude_host              = 1;


        attr->disabled               	= 0; /* enabled */

	switch(aType)
	{
		case LOAD:
		{
			attr->config                 = 0x10002;
			break;
		}
		case STORE:
		default:
		{
        		attr->config                 = 0x10102;
			break;
		}
	}

        attr->read_format            = PERF_FORMAT_GROUP | PERF_FORMAT_ID;
//        attr->task                   = 1;

}

void EBSDevice::unRegisterThisDevice()
{
//	unRegisterDevice();
}

void EBSDevice::deviceOnEvent()
{
	//Event
}
