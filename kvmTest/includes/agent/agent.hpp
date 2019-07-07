#ifndef _AGENT_H_
#define _AGENT_H_


#include <iostream>
#include "context.hpp"
#include "device.hpp"
/*
*
* Class Agent
* An interface class
*
*/

class Agent
{
public:
	Agent(){};
        ~Agent(){};

	Event* agent_parseEvent(void *mmap, int mmap_size,
                        long long *prev_head,
                        int sample_type, int read_format, long long reg_mask,
                        struct validate_values *validate,
                        int quiet, int *events_read,
                        int raw_type);
	void done_processEvent(Event *event, int deviceFileId, int sessionId);

	//Pure virtual function to be imeplemented by MyAgent
	virtual void agent_getCallchain()=0;
	virtual void agent_getAgentContext(Event *event)=0;
	virtual void processEvent(Event *event)=0;
	virtual bool get_mem_access_length_and_type_address(void * ip, uint32_t *accessLen, AccessType *accessType, FloatType * floatType, void * context, void** address)=0;
	virtual void * get_previous_instruction(void *ins, void **pip, void **excludeList, int numExcludes)=0;
	virtual int getValueAtAddress(uint64_t addr, uint8_t *valBuffer, int len)=0;
	virtual bool updateBPAddress(int deviceId, int len, int bpType, uintptr_t addr)=0;
        virtual void registerDevice(Device* dev) = 0;
        virtual void unRegisterDevice(Device* dev) = 0;
	virtual bool pauseDevice(Device* dev) = 0;
	virtual bool resumeDevice(Device* dev) = 0;
	virtual bool isAgentKVM(void) = 0;
	/*-----------------------------------------------------------------------------------------------------------------------------------------------------
	*  If the app decides to notify the platform afterwards, set this true.
	*  Explanation: Sometimes, In app event processing is handled outside of signal handler. 
	*  This requires a NonBlocking call to agent process* requests. With this flags set, process* request will be served after signal handler released
	*  Once the event is processed, the call back callback_process* will be called by the application. done_process* will then call the platform's func 
	*  to report the complete the request.
	*------------------------------------------------------------------------------------------------------------------------------------------------------
	*/
	bool requestWillBeNotified;
};


long long perf_mmap_read( void *mmap, int mmap_size,
                        long long prev_head,
                        int sample_type, int read_format, long long reg_mask,
                        struct validate_values *validate,
                        int quiet, int *events_read,
                        int raw_type, Event* event);



#endif
