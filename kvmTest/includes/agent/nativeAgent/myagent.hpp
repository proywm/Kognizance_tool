#ifndef _MYAGENT_H_
#define _MYAGENT_H_

#include <stddef.h>  // defines NULL
#include <unistd.h>

#include "agent.hpp"
#include "symbol.hpp"

/*
*
* Class MyAgent
* An class
*
*/


class MyAgent : public Agent
{
private:
        MyAgent(){
		char ename[1024];

	        int result = readlink("/proc/self/exe", ename, sizeof(ename));
        	if(result < 0)
        	{
                	std::cout << "failed to find executable\n";
			exit(0);
       		}

		symbol = new Symbol();
		symbol->Load("/proc/self/exe");

		requestWillBeNotified = false;
	}
//        MyAgent(MyAgent const&){};
//        MyAgent& operator=(MyAgent const&){};
        static MyAgent* a_pInstance;
public:
        static MyAgent* Instance();
        void agent_getCallchain();
	void agent_getAgentContext(Event *event);
	void processEvent(Event *event);
	bool get_mem_access_length_and_type_address(void * ip, uint32_t *accessLen, AccessType *accessType, FloatType * floatType, void * context, void** address);
	void * get_previous_instruction(void *ins, void **pip, void **excludeList, int numExcludes);
	int getValueAtAddress(uint64_t addr, uint8_t *valBuffer, int len);
	bool updateBPAddress(int deviceId, int len, int bpType, uintptr_t addr);
        void registerDevice(Device* dev);
        void unRegisterDevice(Device* dev);
	bool pauseDevice(Device* dev);
	bool resumeDevice(Device* dev);
	bool isAgentKVM(void) { return false;}

	Symbol *symbol;
};

#endif
