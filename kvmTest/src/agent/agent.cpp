
#include <iostream>
#include "agent.hpp"
#include "mytool.hpp"

Event* Agent::agent_parseEvent(void *mmap, int mmap_size,
                        long long *prev_head,
                        int sample_type, int read_format, long long reg_mask,
                        struct validate_values *validate,
                        int quiet, int *events_read,
                        int raw_type)
{
#if 0
	Event *event= new Event();
	MyTool *tool = MyTool::Instance();
        event->eventId = tool->generate_event_key();
	*prev_head = perf_mmap_read( mmap, mmap_size, *prev_head, sample_type, read_format, reg_mask, validate, quiet, events_read, raw_type, event);
	agent_getAgentContext(event);
	//accessType and accessLength
	FloatType * floatType = 0;
//	uint64_t pc = event->h_ip_addr;
	void * pc = (void *) event->h_ip_addr;
//	std::cout << "Generic Agent: parsing perf events ==============> ip " << pc << std::endl;
	if(false == get_mem_access_length_and_type_address(pc, (uint32_t*) &(event->accessLength), &(event->accessType), floatType, NULL, NULL))
	{
		event->accessType = UNKNOWN;
		std::cout << "Generic Agent: failed to get accessType and length ================> ip: "<< std::hex << event->h_ip_addr << std::endl;
	}
	else
	{
		std::cout << "Generic Agent: got accessType and length ++++++++++++++++> ip: "<< std::hex << event->h_ip_addr << std::endl;
	}
#endif
	return NULL;
}

void Agent::done_processEvent(Event *event, int deviceFileId, int sessionId)
{
//	std::cout << "Generic Agent: Done agent processing " << std::endl;
	//TODO enable this
	MyTool *tool = MyTool::Instance();
//	Device *dev = tool->get_device(event->deviceFileId);
	Device *dev = tool->get_device(gettid(), deviceFileId);
	if(!dev)
	{
		std::cout << "could not find device, in done process event\n";
	}
	dev->postAgentProcessing(event, -1, NULL, NULL, sessionId);
}
