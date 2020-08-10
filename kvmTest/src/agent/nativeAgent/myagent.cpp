#include <iostream>
#if 0
#define _GNU_SOURCE
#include <ucontext.h>
#include <dlfcn.h>
#include <signal.h>
#include <inttypes.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#endif
#define UNW_LOCAL_ONLY
#include <libunwind.h>

//#include "agent/pythonAgent/myagent.hpp"
#include "myagent.hpp"

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


void MyAgent::agent_retrieveBacktrace(Event *event, ucontext_t *context)
{
	unw_cursor_t cursor; unw_context_t uc;
	unw_word_t ip, sp;

	bool inSignalFrame = true; // Initially we are will observe signal frames

  	unw_getcontext(&uc);
  	unw_init_local(&cursor, &uc);
  	while (unw_step(&cursor) > 0) {
    		unw_get_reg(&cursor, UNW_REG_IP, &ip);
    		unw_get_reg(&cursor, UNW_REG_SP, &sp);
    	//	printf ("ip = %lx, sp = %lx\n", (long) ip, (long) sp);
		if(!inSignalFrame)
		{
			//enter in the event
			ContextFrame *contextFrame = new ContextFrame();
                        contextFrame->binary_addr = ip;
                        contextFrame->method_name = "";
                        contextFrame->source_file = "";
                        event->eventContext_host.push_back(contextFrame);
		}
		if(unw_is_signal_frame(&cursor))
                {
                        inSignalFrame = false;
			//enter the precise ip received from PEBS device
			ContextFrame *contextFrame = new ContextFrame();
                        contextFrame->binary_addr = event->h_ip_addr;
                        contextFrame->method_name = "";
                        contextFrame->source_file = "";
                        event->eventContext_host.push_back(contextFrame);
                }
  	}
}

void MyAgent::processEvent(Event *event)
{
	/*-----------------------------------------------------------------------------------
	* As a native agent, virtual attributes would be same as physical attributes. Copying
	*------------------------------------------------------------------------------------
	*/
	event->ip_addr = event->h_ip_addr;
	event->data_addr = event->h_data_addr;
	event->cpu = event->h_cpu;
	event->pId = event->h_pId;
	event->tId = event->h_tId;
	event->eventContext_agent = event->eventContext_host;
#if 0
	/*-------------------------------------------------------------------------
	* If the event requires to fix IP, it should be done here
	*--------------------------------------------------------------------------
	*/
        //handle agent specific tasks (context, symbol, fixing skid)
	if(event->fixSkid)
        {
		void * contextIP = (void *) event->ip_addr;
		void * patchedIP;
		get_previous_instruction(contextIP, &patchedIP, NULL, 0);
		std::cout << "contextIP " << std::hex << (uint64_t) contextIP << "and patchedIP "<< std::hex<< (uint64_t)patchedIP << "\n";
                event->fixedIP = (uint64_t)patchedIP;
                event->isFixed = true;
        }
#endif
}

bool MyAgent::get_mem_access_length_and_type_address(void * ip, uint32_t *accessLen, AccessType *accessType, FloatType * floatType, void * context, void** address)
{
	return get_mem_access_length_and_type_address__decoder(ip, accessLen, accessType, floatType, context, address);
}

void * MyAgent::get_previous_instruction(void *ins, void **pip, void **excludeList, int numExcludes)
{
	return get_previous_instruction__decoder(ins, pip, excludeList, numExcludes);
}

bool MyAgent::updateBPAddress(int deviceId, int len, int bpType, uintptr_t addr)
{
	return false;
}

void MyAgent::registerDevice(Device* dev)
{

}

void MyAgent::unRegisterDevice(Device* dev)
{

}

bool MyAgent::pauseDevice(Device* dev)
{
        return false;
}

bool MyAgent::resumeDevice(Device* dev)
{
        return false;
}

int MyAgent::getValueAtAddress(uint64_t addr, uint8_t *valBuffer, int len)
{
	return -1;
}
