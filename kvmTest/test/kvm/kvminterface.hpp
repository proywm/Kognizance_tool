#ifndef _KVMINTERFACE_HPP_
#define _KVMINTERFACE_HPP_


#ifdef __cplusplus
extern "C" {
#endif

/*------------------------------
* Redifing Event but as a struct
*-------------------------------
*/
typedef struct
{
	//TODO properly write the event strcut
	int eventId;
}VMEvent;

/*-----------------------------------------------------------------------------------
* KVMInterface: Interface with HyPerf device. Device will push with function pointers
*------------------------------------------------------------------------------------
*/
typedef struct
{
	bool (*getEventState) (VMEvent *event);
}KVMInterface;

/*---------------------------------------------------------------------------
* Empty definition of interface functions. Needed to be over-ridden in HyPerf 
*----------------------------------------------------------------------------
*/
//bool get_mem_access_length_and_type_address__kvm(void * ip, uint32_t *accessLen, AccessType *accessType, FloatType * floatType, void * context, void** address){ return false;}
//void * get_previous_instruction__kvm(void *ins, void **pip, void ** excludeList, int numExcludes){ return NULL;}


bool kvm__register_HyPerf(KVMInterface *kvmInfo);
void kvm__doneEventProcessing_HyPerf(Event *event);

#ifdef __cplusplus
}
#endif


#endif
