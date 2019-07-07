#include <iostream>

#include "myagent.hpp"
#include "pyagent.hpp"

/*
*
* Class MyAgent
* A python agent class
*
*/

// Global static pointer used to ensure a single instance of the class.
MyAgent* MyAgent::a_pInstance = NULL;

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
	std::cout << "PythonAgent: agent_getCallchain" << std::endl;
}

void MyAgent::agent_getAgentContext(Event *event)
{
	//event->eventContext_agent = event->eventContext_host;
	//sig_handler_getContext(&event->eventContext_agent);
	//std::cout << "pythonAgent: agent_getAgentContext" << std::endl;
}

void MyAgent::processEvent(Event *event)
{
        //handle agent specific tasks (context, symbol, fixing skid)
	//agent_getAgentContext(event);
	sig_handler_getContext(event);
}

bool MyAgent::get_mem_access_length_and_type_address(void * ip, uint32_t *accessLen, AccessType *accessType, FloatType * floatType, void * context, void** address)
{
	//return get_mem_access_length_and_type_address(ip, accessLen, accessType, floatType, context, address);
	return false;
}

void * MyAgent::get_previous_instruction(void *ins, void **pip, void ** excludeList, int numExcludes)
{
	return NULL;
}

bool updateBPAddress(int deviceId, int len, int bpType, uintptr_t addr)
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
