#ifndef _MYAGENT_H_
#define _MYAGENT_H_

#include <stddef.h>  // defines NULL
#include <unistd.h>
#include <errno.h>

#include "agent.hpp"
#include "kvminterface.hpp"
#include "symbol.hpp"

#define HBP_NUM 4

/*
*
* Class MyAgent
* An class
*
*/

static char *default_vmlinux[] = {
	"vmlinux",
	"../../../vmlinux",
	"../../vmlinux",
	NULL
}; 

class MyAgent : public Agent
{
private:
        MyAgent(){
		char ename[1024];
 //               int result = readlink(vmlinux, ename, sizeof(ename));
#if 0
		int result = readlink("/home/probir/Downloads/kpv_temp/temp/kvmTest/test/kvm/vmlinux", ename, sizeof(ename));
                if(result < 0)
                {
                        std::cout << "failed to find executable\n"<< errno;
                        exit(0);
                }
#endif
                symbol = new Symbol();
//                hostSymbol->Load("/proc/self/exe");
//		hostSymbol->Load(vmlinux);
		if(!symbol->Load("/home/probir/Downloads/kpv_temp/temp/kvmTest/test/kvm/vmlinux"))
		{
			std::cout << "failed to load symbol table of vmlinux\n"<< errno;
                        exit(0);
		}

		requestWillBeNotified = true;
		//requestWillBeNotified = false;
	};
        static MyAgent* a_pInstance;

	KVMInterface *hyPerfDev = nullptr;
	std::map<int, int> deviceVCPUMap;
	std::map<int, int[HBP_NUM]> VCPUBPdeviceSlotMap;

public:
        static MyAgent* Instance();
	void set_hyPerfDev(KVMInterface *dev);
        void agent_getCallchain();
	void agent_getAgentContext(Event *event);
	void processEvent(Event *event);
	bool get_mem_access_length_and_type_address(void * ip, uint32_t *accessLen, AccessType *accessType, FloatType * floatType, void * context, void** address);
	void * get_previous_instruction(void *ins, void **pip, void **excludeList, int numExcludes);
	void setupPEBSbuffer(void* vshmem_PEBS_ptr);
	bool updateBPAddress(int deviceId, int len, int bpType, uintptr_t addr);
//	int getGuestPhysicalMemory(uint64_t gpa, uint8_t *hostAddr, int len);
	int getValueAtAddress(uint64_t addr, uint8_t *valBuffer, int len);
        void registerDevice(Device* dev);
        void unRegisterDevice(Device* dev);
	bool pauseDevice(Device* dev);
        bool resumeDevice(Device* dev);
	bool isAgentKVM(void) { return true;}

	bool isGuestSample(Event *event);

	int kvm_get_regs(struct kvm_regs* regs);

	Symbol *symbol;
};

#endif
