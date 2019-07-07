#include <iostream>
#include <sys/syscall.h>    /* For SYS_xxx definitions */

#include "kvminterface.hpp"
#include "myagent.hpp"
#include "mytool.hpp"

bool kvm__register_HyPerf(KVMInterface *kvmInfo)
{
	//do what you need to do to register functions
	std::cout << "kvm__register_HyPerf >>>> " << std::endl;
	MyAgent *agent = MyAgent::Instance();
	agent->set_hyPerfDev(kvmInfo);

}

void kvm__VCPU_start(void)
{
        //do what you need to start session
	std::cout << "Within monitor init thread. pid: " << std::dec << getpid() << " pthread_self: "<< pthread_self() << " tid: " <<  syscall(SYS_gettid) << std::endl;
        MyTool *tool = MyTool::Instance();
        tool->tool_threadInit();
}

void kvm__VCPU_end(void)
{
        //do what you need to do end a session
	std::cout << "Within monitor finish thread. pid: " << std::dec << getpid() << " pthread_self: "<< pthread_self() << " tid: " <<  syscall(SYS_gettid) << std::endl;
        MyTool *tool = MyTool::Instance();
        tool->tool_threadExit();
}

void kvm__enableAllEBSDevice(void)
{
//	std::cout << AGENT_MSG_BEGIN << "Guest asking to enabling all EBS devices." << AGENT_MSG_END << std::endl;
	//TODO setup the buffer place
//	MyAgent *agent = MyAgent::Instance();
//	agent->setupPEBSbuffer(vshmem_PEBS_ptr);
        MyTool *tool = MyTool::Instance();
	tool->resume_all_devices();
//	tool->resume_devices_by_session(tool->get_session_key());
}

void kvm__disableAllEBSDevice(void)
{
//	std::cout << AGENT_MSG_BEGIN << "Pausing all EBS devices upon event" << AGENT_MSG_END << std::endl;
	MyTool *tool = MyTool::Instance();
	tool->pause_all_devices();
//	tool->pause_devices_by_session(tool->get_session_key());
}

void kvm__disableEBSDevicesThisSession(unsigned int threadId)
{
	MyTool *tool = MyTool::Instance();
//	tool->get_session_key_from_threadId(threadId);
//	tool->pause_devices_by_session(tool->get_session_key());
	tool->pause_devices_by_session(tool->get_session_key_from_threadId(threadId));
//	std::cout << AGENT_MSG_BEGIN << "Pausing EBS devices for sessions of threadId " << threadId << " session id " << tool->get_session_key_from_threadId(threadId) << AGENT_MSG_END << std::endl;
}
void kvm__enableEBSDevicesThisSession(void)
{
	std::cout << AGENT_MSG_BEGIN << "Guest asking to enabling all EBS devices of this session." << AGENT_MSG_END << std::endl;
	MyTool *tool = MyTool::Instance();
	tool->resume_devices_by_session(tool->get_session_key());
}


void kvm__eventWillbeHandledByOS(bool val)
{
	/*----------------------------------------------------------------------------------------
        * From Now on the driver in guest OS will be resolving the context 
        * So, relase the signal and let the OS run to handle the interrupts.
        *-----------------------------------------------------------------------------------------
        */
	MyAgent *agent = MyAgent::Instance();
	agent->requestWillBeNotified = val;
	std::cout << "kvm__eventWillbeHandledByOS *********** " << std::endl;
}

void kvm__doneEventProcessing_HyPerf(VMEvent *vmEvent, void* contextFrames, int count, unsigned long ip, int deviceId, int threadId,  unsigned long dataAddr, int addressType, int cpu)
{
	MyTool *tool = MyTool::Instance();
	MyAgent *agent = MyAgent::Instance();
	if(ip == 0)
	{
		std::cout << "kvm__doneEventProcessing_HyPerf >>>> IP 0 found: probably sent due to deadlock" << std::hex << ip <<  std::endl;
		agent->done_processEvent(nullptr, deviceId, tool->get_session_key_from_threadId(threadId));
                return;
	}
#if 0
	if(ip >= 0xffffffff810bb1f0 && ip <= 0xffffffff810e0c00)
		return;
#endif

#if 0
	if(tool->get_device(deviceId)->getDeviceType() == PEBSDEVICE)
		std::cout << "PEBS event\n";
	else
		std::cout << "BP event\n";
#endif
#if 0
	if(ip == 0 || ip < 0xfffff00000000000)// 0xffffffff00000000)
	{
		std::cout << "kvm__doneEventProcessing_HyPerf ip is invalid " << std::hex << ip << std::dec << std::endl;
		return;
	}
#endif
#if 0
	std::cout << AGENT_MSG_BEGIN << "<<<<<<<<<<<<<<<< event ID = " << std::dec << evntId << AGENT_MSG_END << std::endl;
	Event* evnt = tool->get_event(evntId);
	if(evnt != nullptr)
		std::cout << "kvm__doneEventProcessing_HyPerf >>>> found event" << std::endl;
	else
		std::cout << "kvm__doneEventProcessing_HyPerf >>>> event not found " << std::endl;
#endif
	if(ip >= 0xffffffff811f9e22 && ip <= 0xffffffff811f9e2b )
	{
		std::cout << "Target ==================================> ip:" << std::hex << ip << " and data addr: " << dataAddr << std::endl;
	}
	switch (addressType)
	{
#ifdef CONSIDER_KERNEL_HEAP
	        case KERNEL_HEAP:
        	{
//	        	std::cout << "Kernel Heap address ip:" << std::hex << ip << " and data addr: " << dataAddr << std::endl;
        	}
		break;
#endif
#ifdef CONSIDER_KERNEL_STACK
                case KERNEL_TASK_STACK:
		case KERNEL_IRQ_STACK:
		case KERNEL_NMI_STACK: 
		case KERNEL_EXCEPTION_STACK:
                {
                        std::cout << "Kernel Stack address ip:" << std::hex << ip << " and data addr: " << dataAddr <<std::endl;
                }
                break;
#endif
		default:
		{
			//skipping this event, just resume the devices
			agent->done_processEvent(nullptr, deviceId, tool->get_session_key_from_threadId(threadId));
			return;
		}
	}
//	if(ip >= 0xffffffff811f9e22 && ip <= 0xffffffff811f9e2b )
//		exit(0);
	std::cout << "Event processing  ==================================> threadId :" << std::dec << threadId << " and deviceId: " << deviceId << std::endl;
	Event *event= new Event();
        event->deviceFileId = deviceId;
        event->eventId = tool->generate_event_key();//evntId;
	event->event_time = (unsigned long long) clock();
	event->sample_index = event->eventId;
	event->h_tId = threadId;
	event->h_data_addr = event->data_addr = dataAddr;
	event->h_ip_addr = event->ip_addr = ip;
	event->addressSpaceType = (AddressSpaceType)addressType;
	event->cpu = cpu;

//	event->isFixed = true;
	event->isFixed = false;

        tool->add_new_event(event->eventId, event);
	unsigned long prev_binary_addr = 0;
	bool startRecording = false;
	int freq = 1;
	for(int iter = 3; iter < count ; iter++)
	{
		VMContextFrame* vmCtx = ((VMContextFrame*)(contextFrames + iter*sizeof(VMContextFrame)));
#if 0
		if(prev_binary_addr != vmCtx->binary_addr)
		{
			if(startRecording)
			{
				//printf("vmCtx %llx iter %d sizeof(VMContextFrame) %d\n",vmCtx, iter, sizeof(VMContextFrame));
				ContextFrame *contextFrame = new ContextFrame();
			        contextFrame->binary_addr = vmCtx->binary_addr;
        			contextFrame->method_name = std::string(vmCtx->method_name);
		           	contextFrame->source_file = "";
        		    	contextFrame->src_lineno = vmCtx->src_lineno;
				event->eventContext_agent.push_back(contextFrame);
				std::cout << "kvm__doneEventProcessing_HyPerf Guest OS Context >>>> " << vmCtx->method_name << "binar addr " << std::hex << vmCtx->binary_addr <<std::endl;
			}
			else
				std::cout << "kvm__doneEventProcessing_HyPerf Guest OS Context (not Included)>>>> " << vmCtx->method_name << "binar addr " << std::hex << vmCtx->binary_addr <<std::endl;
		
		}
#endif
		if(prev_binary_addr == vmCtx->binary_addr)
		{
			freq++;
			if(freq == 2)
			{		
				startRecording = true;
//				std::cout << "kvm__doneEventProcessing_HyPerf Guest OS Context (not Included)>>>> " << vmCtx->method_name << "binar addr " << std::hex << vmCtx->binary_addr <<std::endl;
			}
		}
		if(startRecording)
                {
                                //printf("vmCtx %llx iter %d sizeof(VMContextFrame) %d\n",vmCtx, iter, sizeof(VMContextFrame));
                                ContextFrame *contextFrame = new ContextFrame();
                                contextFrame->binary_addr = vmCtx->binary_addr;
                                contextFrame->method_name = std::string(vmCtx->method_name);
                                contextFrame->source_file = "";
                                contextFrame->src_lineno = vmCtx->src_lineno;
                                event->eventContext_agent.push_back(contextFrame);
//                                std::cout << "kvm__doneEventProcessing_HyPerf Guest OS Context >>>> " << vmCtx->method_name << "binar addr " << std::hex << vmCtx->binary_addr <<std::endl;
                }
		prev_binary_addr = vmCtx->binary_addr;
	}

	// TODO: populate the context frames of Event
	//MyAgent *agent = MyAgent::Instance();
//	agent->done_processEvent(evnt);
	agent->done_processEvent(event, deviceId, tool->get_session_key_from_threadId(threadId));
}

void kvm__doPostProcessing(void)
{
	MyTool *tool = MyTool::Instance();
	tool->tool_doPostProcess();
}
