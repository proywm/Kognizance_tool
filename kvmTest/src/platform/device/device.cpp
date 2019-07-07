#include <sys/syscall.h>    /* For SYS_xxx definitions */

#include "device.hpp"
#include "myagent.hpp"
#include "mytool.hpp"

#define NUM_MMAP_PAGES 8

char * Device::mmap_buffer()
{
	char * buf = static_cast<char*>(mmap(NULL, (1+NUM_MMAP_PAGES) * pgsz, PROT_READ | PROT_WRITE, MAP_SHARED, deviceFileId, 0));
	if (buf == MAP_FAILED) {
                perror("mmap");
//                exit(-1);
		return buf;
	}
	return buf;
}

void Device::unmap_buffer()
{
	CHECK(munmap(mmapBuffer, (1+NUM_MMAP_PAGES) * pgsz));
}

void Device::dev_signal_handler(int signum, siginfo_t *info, void *context)
{
//	std::cout << "Within device handler: calling agent to parse event . current pid " << std::dec << getpid() << " pthread_self: "<< pthread_self() << " tid: " <<  syscall(SYS_gettid) << std::endl;
	/* TODO
		1. Parse event
		2. Construct event Context from agent
		3. Construct native event Context and append with agent context (not decided)
	*/
	MyAgent *agent = MyAgent::Instance();
	//Event *event = agent->agent_parseEvent(mmapBuffer, NUM_MMAP_PAGES, &read_head, pe_attr.sample_type, 0, 0, NULL, quiet, NULL, 0);
	Event *event = handle_perf_event();
	if(deviceType==BPDEVICE)
        {
                event->fixSkid = true;	//asking agent to fix skid
//		std::cout << "Within device handler: BPDevice" << std::endl;
        }

	agent->processEvent(event);
	
	if(!agent->requestWillBeNotified)
	{
		MyTool *tool = MyTool::Instance();
		tool->tool_eventHandler(event, this, signum, info, context);
	}
}

void Device::postAgentProcessing(Event *event, int signum, siginfo_t *info, void *context, int sessionId)
{
	MyAgent *agent = MyAgent::Instance();
	MyTool *tool = MyTool::Instance();
        if(agent->requestWillBeNotified)
	{
//		std::cout << PLATFORM_MSG_BEGIN << "DEVICE: Within postAgentProcessing: calling Tool to handle the event" << PLATFORM_MSG_END << std::endl;
		if(event)
			tool->tool_eventHandler(event, this, signum, info, context);
                //tool->resume_all_devices();
		//tool->resume_devices_by_session(tool->get_session_key());
		tool->resume_devices_by_session(sessionId);
	}
}

Event * Device::handle_perf_event()
{
	Event *event= new Event();
        MyTool *tool = MyTool::Instance();
        event->eventId = tool->generate_event_key();
	event->deviceFileId = deviceFileId;
	int quiet = 1;
        read_head = perf_mmap_read( mmapBuffer, NUM_MMAP_PAGES, read_head, pe_attr.sample_type, 0, 0, NULL, quiet, NULL, 0, event);
	if(event->sample_index != 0)
		event->eventId = event->sample_index;
//	tool->add_new_event(event->eventId, event);
	
	return event;
}

int Device::registerDevice(int sId)
{
	// struct perf_event_attr pe_attr
	memset(&pe_attr, 0, sizeof(struct perf_event_attr));
	registerThisDevice(&pe_attr);
	sessionId = sId;
        // Create the perf_event for current thread on all CPUs with no event group
	if(deviceType==BPDEVICE)
                printf("perf_event_open address %llx\n", pe_attr.bp_addr);
	deviceFileId = perf_event_open(&pe_attr, 0, -1, -1 /*group*/, 0);
        if (deviceFileId == -1) {
	    if(deviceType==BPDEVICE)
            	printf("perf_event_open error: %d -------------------------------------> address %llx\n", deviceFileId, pe_attr.bp_addr);
	    perror("perf_event_open");
//	    exit(-1);
	    return -1;
        }
	// mmap the file 
        mmapBuffer = mmap_buffer();
	if(mmapBuffer == MAP_FAILED)
	{
		mmapBuffer = 0;
		CHECK(close(deviceFileId));
		return -1;
	}

        // Set the perf_event file to async mode
        CHECK(fcntl(deviceFileId, F_SETFL, fcntl(deviceFileId, F_GETFL, 0) | O_ASYNC));

	/* configuring signal for Device */
    	sigset_t block_mask_dev;
    	sigfillset(&block_mask_dev);

   	// Set a signal handler for SIGUSR1
    	struct sigaction sa;
        sa.sa_sigaction = generic_dev_signal_handler;
        sa.sa_mask = block_mask_dev;
        sa.sa_flags = SA_SIGINFO | SA_RESTART | SA_NODEFER | SA_ONSTACK;

	if(sigaction(devSignum,  &sa, 0) == -1) {
        	fprintf(stderr, "Failed to set Device SIG handler: %s\n", strerror(errno));
        	//exit(-1);
		return -1;
	}

        // Tell the device file to send a signal when an event occurs
        CHECK(fcntl(deviceFileId, F_SETSIG, devSignum));

        // Deliver the signal to this thread
        struct f_owner_ex fown_ex;
        fown_ex.type = F_OWNER_TID;
        fown_ex.pid  = getpid();
        int ret = fcntl(deviceFileId, F_SETOWN_EX, &fown_ex);
        if (ret == -1){
            perror("fcntl");
		return -1;
        }
	
	printf("ebs event created %d \n", deviceFileId);
	MyTool *tool = MyTool::Instance();
	tool->add_new_device(deviceFileId, this);
	tool->add_new_device_to_session(deviceFileId, sId);
	
	MyAgent *agent = MyAgent::Instance();
	agent->registerDevice(this);
	
//	isDisabled = 1;
	return 0;
}

void Device::unRegisterDevice()
{
	unRegisterThisDevice();
	unmap_buffer();
	mmapBuffer = 0;
	printf("closing device %d \n", deviceFileId);
	CHECK(close(deviceFileId));
//	close(deviceFileId);
//	MyTool *tool = MyTool::Instance();
  //      tool->remove_device(deviceFileId);
	isDisabled = 0;
	MyAgent *agent = MyAgent::Instance();
	agent->unRegisterDevice(this);
}

void Device::disable_device()
{
	MyAgent *agent = MyAgent::Instance();
        if(!((deviceType==BPDEVICE) && agent->pauseDevice(this)))
		ioctl(deviceFileId, PERF_EVENT_IOC_DISABLE, 0);
}

void Device::enable_device()
{
	MyAgent *agent = MyAgent::Instance();
        if(!((deviceType==BPDEVICE) && agent->resumeDevice(this)))
	{
	//	printf("Not a BP device %d \n", deviceFileId);
		ioctl(deviceFileId, PERF_EVENT_IOC_REFRESH, 1);
	}
}

