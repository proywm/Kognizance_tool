#define _GNU_SOURCE 1
#include "watchpoint.h"
#include "watchpoint_util.h"
#include "watchpoint_mmap.h"
#include <pthread.h>

#include "mytool.hpp"
#include "deadStore.hpp"
#include "redSpy.hpp"
#include "loadSpy.hpp"
#include "dupSpy.hpp"
#include "falsesharing.hpp"
#include "bpDevice.hpp"

#include "myagent.hpp"
//#include "fancyprinting.h"

//*******************Global Variable*****************************/
static WP_Config_t wpConfig;
static pthread_key_t WP_ThreadData_key;

#if 1
bool IsPCSane(void * contextPC, void *possiblePC){
    if( (possiblePC==0) || ((possiblePC > contextPC) ||  (((uint64_t*)contextPC-(uint64_t*)possiblePC) > 15))){
        return false;
    }
    return true;
}

//extern void * get_previous_instruction(void *ins, void **pip, void ** excludeList, int numExcludes);
void *  GetPatchedIP(void *  contextIP) {
    void * patchedIP;
    void * excludeList[MAX_WP_SLOTS] = {0};
    int numExcludes = 0;
    WP_ThreadData_t * tData = TD_GET();
    for(int idx = 0; idx < wpConfig.maxWP; idx++){
        if(tData->watchPointArray[idx].isActive) {
            excludeList[numExcludes]=tData->watchPointArray[idx].va;
            numExcludes++;
        }
    }
//    printf("In get_previous_instruction: numExcludes %d \n", numExcludes);

    MyAgent *agent = MyAgent::Instance();
    agent->get_previous_instruction(contextIP, &patchedIP, excludeList, numExcludes);
    return patchedIP;
}

int GetFloorWPLength(int accessLen) {
    switch (accessLen) {
        default:
        case 8: return 8;
        case 7:case 6: case 5: case 4: return 4;
        case 3:case 2: return 2;
        case 1: return 1;
    }
}

#endif

#if 0
int curWatermarkId = 0;
int pebs_metric_id[NUM_WATERMARK_METRICS] = {-1, -1, -1, -1};

// Actually, only one watchpoint client can be active at a time 
void SetupWatermarkMetric(int metricId) {
	if (curWatermarkId == NUM_WATERMARK_METRICS) {
		ERROR("curWatermarkId == NUM_WATERMARK_METRICS = %d", NUM_WATERMARK_METRICS);
		assert(false);
	}
	/* 
	std::unordered_map<Context *, SampleNum> * propAttrTable = reinterpret_cast<std::unordered_map<Context *, SampleNum> *> (TD_GET(prop_attr_state)[curWatermarkId]);
	if (propAttrTable == nullptr) {
		propAttrTable = new(std::nothrow) std::unordered_map<Context *, SampleNum>();
		assert(propAttrTable);
		TD_GET(ctxt_sample_state)[curWatermarkId] = propAttrTable;
	} 
	*/
	pebs_metric_id[curWatermarkId]=metricId;
	curWatermarkId++;
}

int GetMatchingCtxtSampleTableId(int pebsMetricId) {
	for (int i=0; i<NUM_WATERMARK_METRICS; i++) {
		if(pebs_metric_id[i] == pebsMetricId) return i;
	}
	assert(false);
}


void UpdateNumSamples(Context *ctxt, int pebsMetricId) {
	assert(ctxt);
	int ctxtSampleTableId = GetMatchingCtxtSampleTableId(pebsMetricId);
	std::unordered_map<Context *, SampleNum> *ctxtSampleTable = reinterpret_cast<std::unordered_map<Context *, SampleNum> *>(TD_GET(ctxt_sample_state)[ctxtSampleTableId]);
	if (ctxtSampleTable == nullptr) {
		ctxtSampleTable = new(std::nothrow) std::unordered_map<Context *, SampleNum>();
		assert(ctxtSampleTable);
		TD_GET(ctxt_sample_state)[ctxtSampleTableId] = ctxtSampleTable;
	}

	std::unordered_map<Context *, SampleNum> &myCtxtSampleTable = *ctxtSampleTable;
	if (myCtxtSampleTable.find(ctxt) == myCtxtSampleTable.end()) myCtxtSampleTable[ctxt] = {0, 1};
	else {
		myCtxtSampleTable[ctxt].cur_num++;
	}
}


uint64_t GetNumDiffSamplesAndReset(Context *ctxt, int pebsMetricId, double prop, uint32_t threshold) {
	assert(ctxt);
	int ctxtSampleTableId = GetMatchingCtxtSampleTableId(pebsMetricId);
	std::unordered_map<Context *, SampleNum> *ctxtSampleTable = reinterpret_cast<std::unordered_map<Context *, SampleNum> *>((TD_GET(ctxt_sample_state))[ctxtSampleTableId]);
	assert(ctxtSampleTable);
	/*
	if (ctxtSampleTable == nullptr) {
		ctxtSampleTable = new(std::nothrow) std::unordered_map<Context *, SampleNum>();
		assert(ctxtSampleTable);
		TD_GET(ctxt_sample_state)[ctxtSampleTableId] = ctxtSampleTable;
	} 
	*/
	std::unordered_map<Context *, SampleNum> &myCtxtSampleTable = *ctxtSampleTable;

	double diff = 0., diffWithPeriod = 0.;
	assert(myCtxtSampleTable.find(ctxt) != myCtxtSampleTable.end());
	diff = (myCtxtSampleTable[ctxt].cur_num - myCtxtSampleTable[ctxt].catchup_num) * prop;
	diffWithPeriod = diff * threshold;
	myCtxtSampleTable[ctxt].catchup_num = myCtxtSampleTable[ctxt].cur_num;
	// ERROR("diff = %lf %lf %d %d %lu %lu\n", diff, diffWithPeriod, myCtxtSampleTable.size(), (*ctxtSampleTable).size(), myCtxtSampleTable[ctxt].cur_num, myCtxtSampleTable[ctxt].catchup_num);

	return (uint64_t)diffWithPeriod;
}


#endif


#if 0
static void linux_perf_events_pause(){
   WP_ThreadData_t * tData = TD_GET();
   for (int i = 0; i < wpConfig.maxWP; i++) {
        if(tData->watchPointArray[i].isActive) {
            ioctl(tData->watchPointArray[i].fileHandle, PERF_EVENT_IOC_DISABLE, 0);
        }
   }
}

static void linux_perf_events_resume(){
   WP_ThreadData_t *tData = TD_GET();
   for (int i = 0; i < wpConfig.maxWP; i++) {
        if(tData->watchPointArray[i].isActive) {
            ioctl(tData->watchPointArray[i].fileHandle, PERF_EVENT_IOC_ENABLE, 0);
        }
   }
}
#endif

/**********     WP setting     **********/
static bool ValidateWPData(void * va, int wpLength){
    // Check alignment
#if defined(__x86_64__) || defined(__amd64__) || defined(__x86_64) || defined(__amd64)
    switch (wpLength) {
        case 0: EMSG("\nValidateWPData: 0 length WP never allowed"); return false;
        case 1:
        case 2:
        case 4:
        case 8:
            if(IS_ALIGNED(va, wpLength))
                return true; // unaligned
            else
                return false;
            break;

        default:
            EMSG("Unsuppported WP length %d", wpLength);
            return false; // unsupported alignment
    }
#else
#error "unknown architecture"
#endif
}


static bool IsOveralpped(void *va, int wpLength){
    WP_ThreadData_t * tData = TD_GET();
    // Is a WP with the same/overlapping address active?
    for (int i = 0;  i < wpConfig.maxWP; i++) {
//	printf("IsOveralpped %d of %d \n", i, wpConfig.maxWP);
	if(!tData)
	{
		std::cout << TOOLMSG_BEGIN << " tdata is not valid >>>>>>>>>>>>>>>>> check here "<<TOOLMSG_END << std::endl;
		return false;
	}
        if(tData->watchPointArray[i].isActive){
//	    std::cout << TOOLMSG_BEGIN << " tData->watchPointArray[i].va: "<< std::hex << (uintptr_t)tData->watchPointArray[i].va << " va: " << tData->watchPointArray[i].va << " *(uint64_t *)tData->watchPointArray[i].va " << *(uint64_t *)tData->watchPointArray[i].va <<TOOLMSG_END << std::dec << std::endl;
  //          if(ADDRESSES_OVERLAP(*(uint64_t *)tData->watchPointArray[i].va, tData->watchPointArray[i].wpLength, *(uint64_t *)va, wpLength)){
	      if(ADDRESSES_OVERLAP((uintptr_t)tData->watchPointArray[i].va, tData->watchPointArray[i].wpLength, (uintptr_t)va, wpLength)){
//		std::cout << TOOLMSG_BEGIN << "Overlap found>>>>>>>>>>>>>>>>>  "<<TOOLMSG_END << std::endl;
                return true;
            }
        }
    }
    return false;
}


static inline void EnableWatchpoint(int threadId, int fd) {
    // Start the event
    MyTool *tool = MyTool::Instance();
    Device* dev = tool->get_device(threadId, fd);
    if(!dev)
    {
	std::cout << "could not find device, enabling device failed\n";
	return;
    }
    dev->start_device();
//    CHECK(ioctl(fd, PERF_EVENT_IOC_ENABLE, 0));
}

static inline void DisableWatchpoint(WP_RegisterInfo_t *wpi) {
    // Stop the event
    assert(wpi->fileHandle != -1);
//    CHECK(ioctl(wpi->fileHandle, PERF_EVENT_IOC_DISABLE, 0));
    MyTool *tool = MyTool::Instance();
   
    Device* dev = tool->get_device(wpi->threadId, wpi->fileHandle);
    if(!dev)
    {
        std::cout << "could not find device, disabling device failed\n";
        return;
    }

//    std::cout << TOOLMSG_BEGIN << " DisableWatchpoint "<<TOOLMSG_END << std::endl;
    dev->end_device();
    wpi->isActive = false;
}

static void DisArm(WP_RegisterInfo_t * wpi){

    //assert(wpi->isActive);
    std::cout << "DISARM file handle: " << wpi->fileHandle << std::endl;
    assert(wpi->fileHandle != -1);
    MyTool *tool = MyTool::Instance();
    tool->lockDevice();
    tool->remove_device(wpi->fileHandle);
    tool->unlockDevice();
    wpi->mmapBuffer = 0;

    wpi->fileHandle = -1;
    wpi->isActive = false;
}

void DisableWatchpointWrapper(WP_RegisterInfo_t *wpi){
    if(wpConfig.isWPModifyEnabled) {
	//std::cout << TOOLMSG_BEGIN << " DisableWatchpoint "<<TOOLMSG_END << std::endl;
        DisableWatchpoint(wpi);
    } else {
	//std::cout << TOOLMSG_BEGIN << " DisArm "<<TOOLMSG_END << std::endl;
        DisArm(wpi);
    }
}

static uint64_t oldBadAddress = 0;
void OnWatchPoint(int fd, void *context, Event* event){
#if 0
    linux_perf_events_pause();
#endif
	WP_TriggerAction_t retVal;
	MyTool *tool = MyTool::Instance();
	WitchLogic *wl;
	WP_RegisterInfo_t *wpi = nullptr;
	int location;
	bool false_positive;
	
	switch(tool->logicType)
        {
              	case DEADSPY:
              	{
              		wl = DeadStore::Instance();
                }
                break;
                case REDSPY:
                {
                        wl = RedSpy::Instance();
                }
                break;
		case DUPSPY:
                {
                        wl = DupSpy::Instance();
                }
                break;
                case LOADSPY:
		{
			wl = LoadSpy::Instance();
		}
		break;
		case FALSESHARE:
		{
			wl = FalseSharing::Instance();
		}
		break;
                default:
                {

                }
        }
	//DeadStore *ds = DeadStore::Instance();
	WP_TriggerInfo_t wpt;
	
	WP_ThreadData_t * tData = TD_GET();
	if(!tData)
    	{
		std::cout << ERRMSG_BEGIN << " tdata is not valid: NULL >>>>>>>>>>>>>>>>>>>>>>>>>>>> check here" << std::endl << ERRMSG_END;
		goto cleanup;
    	}

#define SANITIZATION_CHECK
#ifdef SANITIZATION_CHECK
    	false_positive = true;
	for(int i = 0 ; i < wpConfig.maxWP; i++) {
//		std::cout << "sanity check:: fileHandle" << std::dec << tData->watchPointArray[i].fileHandle << " and fd: " << fd << " and cpu: " << event->cpu  <<std::endl;
        	if(fd == tData->watchPointArray[i].fileHandle) {
        		false_positive = false;
            		break;
        	}
    	}
    	if (false_positive) EMSG("\n A false-positive WP trigger in thread %d\n", gettid());

#endif

        std::cout << TOOLMSG_BEGIN << "find which watchpoint fired: ip addr: " << tData->watchPointArray[0].sampledEvent->ip_addr << TOOLMSG_END <<" \n";
    	location = -1;
    	for(int i = 0 ; i < wpConfig.maxWP; i++) {
		if(tool->logicType == DUPSPY)
		{
			if((tData->watchPointArray[i].isActive) && (fd == tData->watchPointArray[i].fileHandle) && (tData->watchPointArray[i].sampledEvent->ip_addr == event->ip_addr)) {
				location = i;
				std::cout << TOOLMSG_BEGIN << "WP addr found: " << std::hex << tData->watchPointArray[i].sampledEvent->ip_addr << " watched addr: " << event->ip_addr << TOOLMSG_END <<" \n";
				break;
			}
		//	else
//				std::cout << "Could not find WP addr: " << std::hex << tData->watchPointArray[i].sampledEvent->ip_addr << " watched addr: " << event->ip_addr << " \n";
		}
		else
		{
        		if((tData->watchPointArray[i].isActive) && (fd == tData->watchPointArray[i].fileHandle) && (tData->watchPointArray[i].sampledEvent->data_addr == event->data_addr)) {
            			location = i;
//			std::cout << "sample addr: " << std::hex << tData->watchPointArray[i].sampledEvent->data_addr << " watched addr: " << event->data_addr << " \n";
	            		break;
        		}
		}
#if 0
		if((tData->watchPointArray[i].sampledEvent->data_addr == event->data_addr))
		{
			if(tData->watchPointArray[i].isActive)
				std::cout << "WP is active and data addr is same. tData->watchPointArray["<< i <<"].fileHandle : " << tData->watchPointArray[i].fileHandle << " and fd: " << fd << std::endl;
			else
				std::cout << "WP is NOT active and data addr is same. tData->watchPointArray[" << i << "].fileHandle : " << tData->watchPointArray[i].fileHandle << " and fd: " << fd << std::endl; 
		}
#endif
    	}
//	std::cout << "----\n";
	
    	if(location == -1){
#if defined WITCHTOOL_DUPSPY
		if(oldBadAddress != event->ip_addr)
#else
		if(oldBadAddress != event->data_addr)
#endif
		{
			std::cout << ERRMSG_BEGIN << " thread " << std::dec << gettid() << " WP trigger did not match any known active WP : Data Addr: " << std::hex << event->data_addr << " detected fd that did not match: " << std::dec << fd << " and wpConfig.maxWP: " << wpConfig.maxWP <<ERRMSG_END << std::endl;
			for(int i = 0 ; i < wpConfig.maxWP; i++) {
				if((tData->watchPointArray[i].sampledEvent->data_addr == event->data_addr))
                		{
		                        if(tData->watchPointArray[i].isActive)
                                		std::cout << "WP is active and data addr is same. tData->watchPointArray["<< i <<"].fileHandle : " << tData->watchPointArray[i].fileHandle << " and fd: " << fd << std::endl;
                        		else
                                		std::cout << "WP is NOT active and data addr is same. tData->watchPointArray[" << i << "].fileHandle : " << tData->watchPointArray[i].fileHandle << " and fd: " << fd << std::endl; 
                		}
			}
#if defined WITCHTOOL_DUPSPY
			oldBadAddress = event->ip_addr;
#else
			oldBadAddress = event->data_addr;
#endif
		}
		exit(0);
		goto cleanup;
    	}
//	std::cout << "BP happened\n";
    	wpi = &tData->watchPointArray[location];

//    DisableWatchpointWrapper(&tData->watchPointArray[location]);

    	wpt.va = wpi->va;
    	wpt.watchLen = wpi->wpLength;
//    wpt.ctxt = context;
    	wpt.pc = 0; //TODO: how to get pc?

	if(event->h_ip_addr == 0)
        {
                std::cout << ERRMSG_BEGIN << " event->h_ip_addr is found to be 0 >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> Error occurred . find file and delete data address from watchlist" << std::endl << ERRMSG_END;
                retVal = WP_DISABLE;
                goto SkipWPHandle;
        }

//    wpt.data = wpi->userData;
  //  printf("Collecting trigger info \n");

//    if (WP_CollectTriggerInfo(wpi, &wpt, context, wpConfig.pgsz)){
//	printf("Collected\n");
//    	retVal = tData->fptr(&wpt);
  //  } else {
	// drop this sample
//	retVal = WP_DISABLE;
    //}

	if(!event)
	{
                std::cout << ERRMSG_BEGIN << "WITCH: Within BP: EVENT IS NULL" << ERRMSG_END << std::endl;
		retVal = WP_DISABLE;
                goto SkipWPHandle;
	}
        //std::cout << TOOLMSG_BEGIN << "WICTH: Within BP handler: Doing tool specific handling. "<< std::hex<<event->h_data_addr << event->eventId << TOOLMSG_END << std::endl;

        //Fixing IP
        if(!(event->isFixed || tool->fixIP(event)))
        {
                std::cout << ERRMSG_BEGIN <<"WITCH: Failed to fix IP. Might impact on WitchTool" << ERRMSG_END << std::endl;
                retVal = WP_DISABLE;
                goto SkipWPHandle;
        }


    	retVal = wl->Witch_OnWP_Callback(event, wpi->sampledEvent, &wpt);
//    	retVal = WP_DISABLE;

SkipWPHandle:
//    	printf("OnWatchPoint retval: %d \n", retVal);

    	switch (retVal) {
        	case WP_DISABLE: {
            		if(wpi->isActive){
				printf("WP disabling\n");
                		DisableWatchpointWrapper(wpi);
            		}
            //reset to tData.samplePostFull
		        tData->samplePostFull = SAMPLES_POST_FULL_RESET_VAL;
            // Reset per WP probability
            		wpi->samplePostFull = SAMPLES_POST_FULL_RESET_VAL;
        	}
	        break;
        	case WP_DISABLE_ALL: {
	            for(int i = 0; i < wpConfig.maxWP; i++) {
        	        if(tData->watchPointArray[i].isActive){
                	    DisableWatchpointWrapper(&(tData->watchPointArray[i]));
                	}
                // Reset per WP probability
	                tData->watchPointArray[i].samplePostFull = SAMPLES_POST_FULL_RESET_VAL;
        	    }
            //reset to tData.samplePostFull to SAMPLES_POST_FULL_RESET_VAL
	            tData->samplePostFull = SAMPLES_POST_FULL_RESET_VAL;
        	}
	        break;
        	case WP_ALREADY_DISABLED: { // Already disabled, perhaps in pre-WP action
	            assert(wpi->isActive == false);
        	    tData->samplePostFull = SAMPLES_POST_FULL_RESET_VAL;
            // Reset per WP probability
	            wpi->samplePostFull = SAMPLES_POST_FULL_RESET_VAL;
        	}
	        break;
        	case WP_RETAIN: { // resurrect this wp
		    //printf("WP Retain\n");
	            if(!wpi->isActive){
			// printf("WP Retain: not enabled... going to enable that\n");
        	        EnableWatchpoint(gettid(), wpi->fileHandle);
                	wpi->isActive = true;
	            }
		    //dont cleanup.. just return;
	            return;
        	}
	        break;
        	default: // Retain the state
	            break;
    	}
	std::cout << "BP handled\n";

    //}
#if 0
    linux_perf_events_resume();
#endif
	//delete both event
cleanup:
//	printf("Deleting watched event id %d \n", event->eventId);
	if(event != nullptr)
		delete event;

//	printf("Deleting sampled event id %d \n", wpi->sampledEvent->eventId);
	if(wpi != nullptr && wpi->sampledEvent != nullptr)
		delete wpi->sampledEvent;
//	printf("Deleted all \n");

    return;
}

volatile int dummyWP[100];

static void CreateWatchPoint(WP_RegisterInfo_t *wpi, int watch_length, WP_Access_t watch_type, void * va, bool modify, Event* sampleEvent) {
    // Perf event settings
#if 0
    struct perf_event_attr pe;

    memset(&pe, 0, sizeof(struct perf_event_attr));
    pe.type                   = PERF_TYPE_BREAKPOINT;
    pe.size                   = sizeof(struct perf_event_attr);
    //        .bp_type                = HW_BREAKPOINT_W,
    //        .bp_len                 = HW_BREAKPOINT_LEN_4,
    pe.sample_period          = 1;
    pe.sample_type            = (PERF_SAMPLE_IP );
    //pe.branch_sample_type = (PERF_SAMPLE_BRANCH_ANY);
    pe.disabled               = 0; /* enabled */
    pe.exclude_user           = 0;
    pe.exclude_kernel         = 1;
    pe.exclude_hv             = 1;


    pe.precise_ip             = wpConfig.isLBREnabled? 2 /*precise_ip 0 skid*/ : 0 /* arbitraty skid */;
#endif
    int bp_len;
    int bp_type;
    uintptr_t bp_addr;
    switch (watch_length) {
        case 1: bp_len = HW_BREAKPOINT_LEN_1; break;
        case 2: bp_len = HW_BREAKPOINT_LEN_2; break;
        case 4: bp_len = HW_BREAKPOINT_LEN_4; break;
        case 8: bp_len = HW_BREAKPOINT_LEN_8; break;
        default:
            EMSG("Unsupported .bp_len %d: %s\n", watch_length,strerror(errno));
            exit(0);
    }
    bp_addr = (uintptr_t)va;
    switch (watch_type) {
	case WP_INST: bp_type = HW_BREAKPOINT_X; break;
        case WP_READ: bp_type = HW_BREAKPOINT_R; break;
        case WP_WRITE: bp_type = HW_BREAKPOINT_W; break;
        default: bp_type = HW_BREAKPOINT_W | HW_BREAKPOINT_R;
    }
//    printf("CreateWatchPoint >>>>>>>>>>>>>>>>>>>>>>>>>>>> %llx , watch_type %d\n", bp_addr, watch_type);
//#if defined(FAST_BP_IOC_FLAG)
    if(modify) {
	if(wpi == nullptr)
		printf("CreateWatchPoint >>>>>>>>>>>>>>>>>>>>>>>>>>>> %llx -- wpi is null\n", bp_addr);
        // modification
        assert(wpi->fileHandle != -1);
        //assert(wpi->mmapBuffer != 0);
        DisableWatchpoint(wpi);
    //    CHECK(ioctl(wpi->fileHandle, FAST_BP_IOC_FLAG, (unsigned long) (&pe)));
	//call bp device's update function
	std::cout << std::dec << "threadId " << wpi->threadId << " and wpi->fileHandle: " << wpi->fileHandle << std::endl;
//	return ;
	MyTool *tool = MyTool::Instance();
	Device *d = tool->get_device(wpi->threadId, wpi->fileHandle);
	if(!d)
	{
		printf("failed to find the device >>>>>>>>>>>>>>>>>>>>>>>>>>>> %llx \n", bp_addr);
                return;
	}

	BPDevice* dev = dynamic_cast<BPDevice*> (tool->get_device(wpi->threadId, wpi->fileHandle));
	if(!dev->updateBPAddress(bp_len, bp_type, (uintptr_t)bp_addr))
	{
		printf("failed to update breakpoint >>>>>>>>>>>>>>>>>>>>>>>>>>>> %llx \n", bp_addr);
		return;
		//exit(0);
	}
//	else
//		printf("Success to update breakpoint >>>>>>>>>>>>>>>>>>>>>>>>>>>> \n");
        if(wpi->isActive == false) {
            EnableWatchpoint(wpi->threadId, wpi->fileHandle);
        }
    } else
//#endif
    {
//	bp_addr = (uintptr_t)&dummyWP[0];
//	printf("CreateWatchPoint >>>>>>>>>>>>>>>>>>>>>>>>>>>> %llx -- 1\n", bp_addr);
	BPDevice *dev = new BPDevice( bp_len, bp_type, (uintptr_t)bp_addr);
#if 0
        // fresh creation
        // Create the perf_event for this thread on all CPUs with no event group
        int perf_fd = perf_event_open(&pe, 0, -1, -1 /*group*/, 0);
        if (perf_fd == -1) {
            EMSG("Failed to open perf event file: %s\n",strerror(errno));
            exit(0);
        }
        // Set the perf_event file to async mode
        CHECK(fcntl(perf_fd, F_SETFL, fcntl(perf_fd, F_GETFL, 0) | O_ASYNC));
        // Tell the file to send a signal when an event occurs
        CHECK(fcntl(perf_fd, F_SETSIG, wpConfig.signalDelivered));
        // Deliver the signal to this thread
        struct f_owner_ex fown_ex;
        fown_ex.type = F_OWNER_TID;
        fown_ex.pid  = gettid();
        int ret = fcntl(perf_fd, F_SETOWN_EX, &fown_ex);
        if (ret == -1){
            EMSG("Failed to set the owner of the perf event file: %s\n", strerror(errno));
            return;
        }
        //       CHECK(fcntl(perf_fd, F_SETOWN, gettid()));

        wpi->fileHandle = perf_fd;
        // mmap the file if lbr is enabled
        if(wpConfig.isLBREnabled) {
            wpi->mmapBuffer = WP_MapBuffer(perf_fd, wpConfig.pgsz);
        }
#endif
	
	MyTool *tool = MyTool::Instance();
	int sessionId = tool->get_session_key();
//	printf("CreateWatchPoint >>>>>>>>>>>>>>>>>>>>>>>>>>>> %llx -- 2\n", bp_addr);
	if(0 != dev->registerDevice(sessionId))
	{
		wpi->fileHandle = -1;
		return;
	}
	printf("CreateWatchPoint >>>>>>>>>>>>>>>>>>>>>>>>>>>> %llx -- 3\n", dev->getDeviceFildId());
	wpi->fileHandle = dev->getDeviceFildId();
	wpi->mmapBuffer = dev->getDeviceMMapBuffer();
    }
  //  printf("CreateWatchPoint >>>>>>>>>>>>>>>>>>>>>>>>>>>> %llx -- 4\n", bp_addr);
    wpi->isActive = true;
    wpi->va = (void *) bp_addr;
    wpi->wpLength = watch_length;
    if(sampleEvent)
	    wpi->sampledEvent = sampleEvent;
    else
	printf("Failed to set sampledEvent >>>>>>>>>>>>>>>>>>>>>>>>>>>> %llx -- 4\n", bp_addr);
//    wpi->watchLen = watchLen;
//    wpi->sampleAccessLen = accessLen;
    wpi->startTime = rdtsc();
}


static bool ArmWatchPoint(WP_RegisterInfo_t * wpi, int watch_length, WP_Access_t watch_type, void * va, Event *sampleEvent) {
    // if WP modification is suppoted use it
    if(wpConfig.isWPModifyEnabled){
        // Does not matter whether it was active or not.
        // If it was not active, enable it.
        if(wpi->fileHandle != -1) {
	    std::cout << TOOLMSG_BEGIN << " wpConfig.isWPModifyEnabled " << TOOLMSG_END << std::endl;
            CreateWatchPoint(wpi, watch_length, watch_type, va, true, sampleEvent);
            return true;
        }
    }

    // disable the old WP if active
    if(wpi->isActive) {
	std::cout << TOOLMSG_BEGIN << " wpi->isActive " << TOOLMSG_END << std::endl;
        DisArm(wpi);
    }
//    std::cout << TOOLMSG_BEGIN << " calling CreateWatchPoint " << TOOLMSG_END << std::endl;
    CreateWatchPoint(wpi, watch_length, watch_type, va, false, sampleEvent);
    if(wpi->fileHandle == -1)
	return false;
    return true;
}







// Finds a victim slot to set a new WP
static WP_Victim_t GetVictim(int * location, WP_ReplacementPolicy_t policy){
    WP_ThreadData_t *tData = TD_GET();
    // If any WP slot is inactive, return it;
    for(int i = 0; i < wpConfig.maxWP; i++){
//	if(tData->watchPointArray[i].isActive)
//		cout << "watchPointArray[" << i << "].event.data_addr: " << tData->watchPointArray[i].sampledEvent->data_addr << " tData->watchPointArray[i].fileHandle " << tData->watchPointArray[i].fileHandle << std::endl;
        if(!tData->watchPointArray[i].isActive) {
            *location = i;
            // Increase samplePostFull for those who survived.
            for(int rest = 0; rest < wpConfig.maxWP; rest++){
                 if (tData->watchPointArray[rest].isActive) {
                     tData->watchPointArray[rest].samplePostFull++;
//		     printf(" wp slot[%d] increasing samplePostFull\n", rest);
                 }
            }
            return WP_VICTIM_EMPTY_SLOT;
        }
//	else
	{
//		printf(" wp slot[%d] is active\n", i);
	}
	
    }
    switch (policy) {
        case WP_REPLACEMENT_AUTO:{
#if 0  // reservoir sampling with shared probablity
            // Equal probability for any data access


            // Randomly pick a slot to victimize.
            long int tmpVal;
            lrand48_r(&tData->randBuffer, &tmpVal);
            int rSlot = tmpVal % wpConfig.maxWP;
            *location = rSlot;

            // if it is the first sample after full, use wpConfig.maxWP/(wpConfig.maxWP+1) probability to replace.
            // if it is the second sample after full, use wpConfig.maxWP/(wpConfig.maxWP+2) probability to replace.
            // if it is the third sample after full, use wpConfig.maxWP/(wpConfig.maxWP+3) probability replace.

            double probabilityToReplace =  wpConfig.maxWP/((double)wpConfig.maxWP+tData->samplePostFull);
            double randValue;
            drand48_r(&tData->randBuffer, &randValue);
            // update tData.samplePostFull
            tData->samplePostFull++;

            if(randValue <= probabilityToReplace) {
                return WP_VICTIM_NON_EMPTY_SLOT;
            }
            // this is an indication not to replace, but if the client chooses to force, they can
            return WP_VICTIM_NONE_AVAILABLE;
#else //reservoir sampling with individual probablity
            // Shuffle the visit order
            int slots[MAX_WP_SLOTS];
            for(int i = 0; i < wpConfig.maxWP; i++)
                slots[i] = i;
            // Shuffle
            for(int i = 0; i < wpConfig.maxWP; i++){
                long int randVal;
                lrand48_r(&(tData->randBuffer), &randVal);
                randVal = randVal % wpConfig.maxWP;
                int tmp = slots[i];
                slots[i] = slots[randVal];
                slots[randVal] = tmp;
            }

            // attempt to replace each WP with its own probability
            for(int i = 0; i < wpConfig.maxWP; i++) {
                int loc = slots[i];
                double probabilityToReplace =  1.0/(1.0 + (double)tData->watchPointArray[loc].samplePostFull);
                double randValue;
                drand48_r(&(tData->randBuffer), &randValue);

                // update tData.samplePostFull
                tData->watchPointArray[loc].samplePostFull++;
//		printf("randValue %lf probabilityToReplace %lf samplePostFull %d\n", randValue, probabilityToReplace, tData->watchPointArray[loc].samplePostFull);

                if(randValue <= probabilityToReplace) {
                    *location = loc;
                    for(int rest = i+1; rest < wpConfig.maxWP; rest++){
                        tData->watchPointArray[slots[rest]].samplePostFull++;
                    }
                    return WP_VICTIM_NON_EMPTY_SLOT;
                }
                // TODO: Milind: Not sure whether I should increment samplePostFull of the remainiing slots.
            }
            // this is an indication not to replace, but if the client chooses to force, they can
            *location = slots[0] /*random value*/;
//	    printf(" WP_VICTIM_NONE_AVAILABLE\n");
            return WP_VICTIM_NONE_AVAILABLE;
#endif
        }
            break;

        case WP_REPLACEMENT_NEWEST:{
            // Always replace the newest

            int64_t newestTime = 0;
            for(int i = 0; i < wpConfig.maxWP; i++){
                if(newestTime < tData->watchPointArray[i].startTime) {
                    *location = i;
                    newestTime = tData->watchPointArray[i].startTime;
                }
            }
            return WP_VICTIM_NON_EMPTY_SLOT;
        }
            break;
        case WP_REPLACEMENT_OLDEST:{
            // Always replace the oldest

            int64_t oldestTime = INT64_MAX;
            for(int i = 0; i < wpConfig.maxWP; i++){
                if(oldestTime > tData->watchPointArray[i].startTime) {
                    *location = i;
			    oldestTime = tData->watchPointArray[i].startTime;
			}
		    }
		    return WP_VICTIM_NON_EMPTY_SLOT;
		}
		    break;

		case WP_REPLACEMENT_EMPTY_SLOT:{
		    return WP_VICTIM_NONE_AVAILABLE;
		}
		    break;
		default:
		    return WP_VICTIM_NONE_AVAILABLE;
	    }
	    // No unarmed WP slot found.
	}


	//*************************************Interface Implementation*****************************//

int WP_Init()
{
	if ( pthread_key_create(&WP_ThreadData_key, NULL) != 0){
		EMSG("Failed to pthread_key_create: %s\n", strerror(errno));
		return -1;
	}

	wpConfig.isLBREnabled = true;
  	MyAgent *agent = MyAgent::Instance();
        if(agent->isAgentKVM())
	{
		wpConfig.isWPModifyEnabled = true;
	}
	else
	{
#if defined(FAST_BP_IOC_FLAG)
		wpConfig.isWPModifyEnabled = true;
#else
		wpConfig.isWPModifyEnabled = false;
#endif
	}
	
	//    wpConfig.signalDelivered = SIGRTMIN + 3;

	wpConfig.pgsz = sysconf(_SC_PAGESIZE);

	MyTool *tool = MyTool::Instance();
	wpConfig.maxWP = tool->getMaxDevices(BPDEVICE);

	wpConfig.replacementPolicy = WP_REPLACEMENT_AUTO;
//	wpConfig.replacementPolicy = WP_REPLACEMENT_OLDEST;
	wpConfig.userPerfPause = NULL;
	wpConfig.userPerfResume = NULL;

	return 0;
}

void WP_Shutdown(){

}



	// Per thread initialization
int WP_ThreadInit( WP_TrapCallback_t cb_func){
	std::cout << TOOLMSG_BEGIN << " +++++++++++++++++++++++++++++++++++++++++++++++++++++ " << TOOLMSG_END << std::endl;
#if 0
	if ( pthread_key_create(&WP_ThreadData_key, NULL) != 0){
                EMSG("Failed to pthread_key_create: %s\n", strerror(errno));
                return -1;
            }
#endif
	    // setup the thread local data
	    WP_ThreadData_t *tData = (WP_ThreadData_t *)malloc(sizeof(WP_ThreadData_t));
	    if (tData == NULL){
		EMSG("Failed to malloc thread local data\n");
	//	exit(0);
		return -1;
	    }

	    if (pthread_setspecific(WP_ThreadData_key, (void *) tData) != 0){
		EMSG("Failed to pthread_setspecific: %s\n", strerror(errno));
	//	exit(0);
		return -1;
	    }
	    
	    tData->lbrDummyFD = -1;
	    tData->fptr = cb_func;
	    tData->fs_reg_val = (void*)-1;
	    tData->gs_reg_val = (void*)-1;
	    srand48_r(time(NULL), &tData->randBuffer);
	    tData->samplePostFull = SAMPLES_POST_FULL_RESET_VAL;
	    tData->numWatchpointTriggers = 0;
	    tData->numWatchpointImpreciseIP = 0;
	    tData->numWatchpointImpreciseAddressArbitraryLength = 0;
	    tData->numWatchpointImpreciseAddress8ByteLength = 0;
	    tData->numWatchpointDropped = 0;
	    tData->numSampleTriggeringWatchpoints = 0;
	    tData->numInsaneIP = 0;


	    for (int i=0; i<wpConfig.maxWP; i++) {
		tData->watchPointArray[i].isActive = false;
	        tData->watchPointArray[i].fileHandle = -1;
        	tData->watchPointArray[i].startTime = 0;
        	tData->watchPointArray[i].samplePostFull = SAMPLES_POST_FULL_RESET_VAL;
		tData->watchPointArray[i].threadId = gettid();
    		}

    //if LBR is supported create a dummy PERF_TYPE_HARDWARE for Linux workaround
    /*if(wpConfig.isLBREnabled) {
        CreateDummyHardwareEvent();
    }*/
    return 0;
}

void WP_ThreadTerminate(){
    WP_ThreadData_t *tData = (WP_ThreadData_t *)pthread_getspecific(WP_ThreadData_key);
    pthread_setspecific(WP_ThreadData_key,NULL);

#if 1
    for (int i = 0; i < wpConfig.maxWP; i++) {
        if(tData->watchPointArray[i].fileHandle != -1) {
            DisArm(&tData->watchPointArray[i]);
        }
    }
#endif
    /*
    if(tData.lbrDummyFD != -1) {
        CloseDummyHardwareEvent(tData.lbrDummyFD);
        tData.lbrDummyFD = -1;
    }*/
    tData->fs_reg_val = (void*)-1;
    tData->gs_reg_val = (void*)-1;
#if 0
    free(tData->ss.ss_sp);
#endif
    free(tData);
}

int WP_Subscribe(void * va, int watch_length, int accessLen, WP_Access_t watch_type, Event *sampleEvent){
    if(watch_type != WP_INST && ValidateWPData(va, watch_length) == false) {
	printf("Not Valid \n");
        return -1;
    }
//    printf("WP address validated\n");
    if(IsOveralpped(va, watch_length)){
	printf("is overlapped \n");
        return -1; // drop the sample if it overlaps an existing address
    }
//    printf("WP address is not overlapped\n");
    WP_ThreadData_t *tData = TD_GET();
    if(!tData)
    {
        std::cout << ERRMSG_BEGIN << " tdata is not valid: NULL >>>>>>>>>>>>>>>>>>>>>>>>>>>> check here" << std::endl << ERRMSG_END;
        return -1;
    }
    // No overlap, look for a victim slot
    int victimLocation = -1;
    // Find a slot to install WP
    WP_Victim_t r = GetVictim(&victimLocation, wpConfig.replacementPolicy);
    if(r != WP_VICTIM_NONE_AVAILABLE) {
//	printf("victimLocation %d and tData->watchPointArray[victimLocation].fileHandle %d tData->lbrDummyFD %d\n", victimLocation, tData->watchPointArray[victimLocation].fileHandle, tData->lbrDummyFD);
//	if(data != nullptr)
//	{
		printf("victimLocation %d \n", victimLocation);
//	        tData->watchPointArray[victimLocation].userData = data;
//	}
	if(tData->watchPointArray[victimLocation].isActive)
		printf("WP_Subscribe %d. tData->watchPointArray[victimLocation].fileHandle  active? Yes .. addr %llx \n", victimLocation, tData->watchPointArray[victimLocation].va);//, tData->watchPointArray[victimLocation].fileHandle);
	else
		printf("WP_Subscribe %d. tData->watchPointArray[victimLocation].fileHandle  active? No .. addr %llx\n", victimLocation, tData->watchPointArray[victimLocation].va);//, tData->watchPointArray[victimLocation].fileHandle);
        if(ArmWatchPoint(&(tData->watchPointArray[victimLocation]), watch_length, watch_type, va, sampleEvent) == false){
            //LOG to hpcrun log
            EMSG("ArmWatchPoint failed for address %p\n", va);
            return -1;
        }
//	printf("WP found victim %d \n", victimLocation);
        return 0;
    }
//    else
//    	printf("WP_VICTIM_NONE_AVAILABLE \n");
    return -1;
}


void WP_SetPerfPauseAndResumeFunctions(WP_PerfCallback_t pause_fn,  WP_PerfCallback_t resume_fn){
    wpConfig.userPerfPause = pause_fn;
    wpConfig.userPerfResume = resume_fn;
}

