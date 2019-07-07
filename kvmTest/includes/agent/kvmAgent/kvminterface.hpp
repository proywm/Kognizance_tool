#ifndef _KVMINTERFACE_HPP_
#define _KVMINTERFACE_HPP_


#ifdef __cplusplus
extern "C" {
#endif
#include <linux/kvm.h>

/*------------------------------
* Redifing Event but as a struct
*-------------------------------
*/
typedef struct
{
	//TODO properly write the event strcut
	int eventId;
	int h_tId;
}VMEvent;

/*----------------------------------------------------------------------
* Redifing ContextFrame but as a struct. Compiler padding is not desired
*-----------------------------------------------------------------------
*/
#define MAXCONTEXTFRAMES 16
#define METHODNAMELENGTH 128
//KSYM_SYMBOL_LEN

#pragma pack(push, 1)
typedef struct {
        unsigned long long binary_addr;
        char method_name[METHODNAMELENGTH];
//        char source_file[METHODNAMELENGTH];
        int src_lineno;
}VMContextFrame;
#pragma pack(pop)

/*-----------------------------------------------------------------------------------
* KVMInterface: Interface with HyPerf device. Device will push with function pointers
*------------------------------------------------------------------------------------
*/
typedef struct
{
	void *dev;
	bool (*getEventState) (void *dev, VMEvent *event);
	int (*getVCPU)(void);
	bool (*updateBPAddress) (int vcpu_fd, int len, int bpType, uintptr_t addr, int breakno);
	int (*getGuestPhysicalMemory) (uint64_t gpa, uint8_t *hostAddr, int len);
	bool (*pauseBP) (int vcpu_fd, int len, int bpType, int breakno);
	bool (*resumeBP) (int vcpu_fd, int len, int bpType, int breakno);
	int (*kvm_get_regs) (struct kvm_regs* regs);
}KVMInterface;

/*---------------------------------------------------------------------------
* Empty definition of interface functions. Needed to be over-ridden in HyPerf 
*----------------------------------------------------------------------------
*/
//bool get_mem_access_length_and_type_address__kvm(void * ip, uint32_t *accessLen, AccessType *accessType, FloatType * floatType, void * context, void** address){ return false;}
//void * get_previous_instruction__kvm(void *ins, void **pip, void ** excludeList, int numExcludes){ return NULL;}


bool kvm__register_HyPerf(KVMInterface *kvmInfo);
void kvm__VCPU_start(void);
void kvm__VCPU_end(void);
void kvm__enableAllEBSDevice(void);
void kvm__disableAllEBSDevice(void);
void kvm__disableEBSDevicesThisSession(unsigned int threadId);
void kvm__enableEBSDevicesThisSession(void);
void kvm__eventWillbeHandledByOS(bool val);
void kvm__doneEventProcessing_HyPerf(VMEvent *event, void* contextFrames, int count, unsigned long ip, int deviceId, int threadId, unsigned long dataAddr, int addressType, int cpu);
void kvm__doPostProcessing(void);

#ifdef __cplusplus
}
#endif


#endif
