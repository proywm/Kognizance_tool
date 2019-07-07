#include <stddef.h>  // defines NULL
#include "falsesharing.hpp"
#include "watchpoint_util.h"
#include "logger.hpp"


// Global static pointer used to ensure a single instance of the class.
FalseSharing* FalseSharing::ds_pInstance = NULL;

thread_local uint64_t FalseSharing::totalDeadBytes_perThread = 0;
thread_local uint64_t FalseSharing::totalUsedBytes_perThread = 0;
thread_local uint64_t FalseSharing::watchedSampleCount_perThread = 0;

uint64_t FalseSharing::totalDeadBytes = 0;
uint64_t FalseSharing::totalUsedBytes = 0;
uint64_t FalseSharing::watchedSampleCount = 0;

//extern struct propAttrTablecomp_By_MetricVal propAttrTable_SortBy_MetricVal;
extern struct propAttrTablecomp_By_Sample propAttrTable_SortBy_Sample;
extern struct propAttrTablecomp_By_EventDistance propAttrTable_SortBy_EventDistance;
extern struct propAttrTablecomp_By_MetricVal_USEDBYTES propAttrTable_SortBy_MetricVal_USEDBYTES;
extern struct propAttrTablecomp_By_MetricVal_REDBYTES propAttrTable_SortBy_MetricVal_REDBYTES;
extern struct propAttrTablecomp_By_MetricVal_APPROXREDBYTES propAttrTable_SortBy_MetricVal_APPROXREDBYTES;
extern struct propAttrTablecomp_By_MetricVal_DEADBYTES propAttrTable_SortBy_MetricVal_DEADBYTES;
extern struct propAttrTablecomp_By_MetricVal_SAMPLECOUNT propAttrTable_SortBy_MetricVal_SAMPLECOUNT;
extern struct propAttrTablecomp_By_MetricVal_WATCHEDCOUNT propAttrTable_SortBy_MetricVal_WATCHEDCOUNT;
extern struct propAttrTablecomp_By_MetricVal_INEFFICIENCY propAttrTable_SortBy_MetricVal_INEFFICIENCY;

/** This function is called to create an instance of the class. 
    Calling the constructor publicly is not allowed. The constructor 
    is private and is only called by this Instance function.
*/
  
FalseSharing* FalseSharing::Instance()
{
   if (!ds_pInstance)   // Only allow one instance of class to be generated.
      ds_pInstance = new FalseSharing;

   return ds_pInstance;
}


//WP_TriggerAction_t FalseSharing::Witch_OnWP_Callback(WP_TriggerInfo_t *wpi){
WP_TriggerAction_t FalseSharing::Witch_OnWP_Callback(Event *currEvent, Event *sampleEvent, WP_TriggerInfo_t *wpt){
	contextTree_mtx.lock();

	if(currEvent->accessType == UNKNOWN)// && currEvent->accessType == LOAD)
	{
		removeEventContextMapEntry(sampleEvent);
		contextTree_mtx.unlock();
		return WP_DISABLE;
	}

	auto leafContextIter = eventToContextLeafMap.find(sampleEvent->eventId);
	if(leafContextIter != eventToContextLeafMap.end())
	{
	
//		std::cout << TOOLMSG_BEGIN << "Witch: Got leaf node of context tree " << TOOLMSG_END << std::endl;

//		Context* leafContext = addFramesToContextTree(currEvent->eventContext_agent, leafContextIter->second, false);
		//update proportional attibution table.
		double myProp = 1.0;
		uint32_t threshold = 100003;//update this value from param
	
		int64_t numDiffSamples = GetNumDiffSamplesAndReset(leafContextIter->second, myProp, threshold, (currEvent->event_time - sampleEvent->event_time));
		int64_t inc = numDiffSamples * sampleEvent->accessLength;//wpt->sampleAccessLen;

		// Only if the addresses do NOT overlap, do we use the sample address!
//		void *sampleAddr = sampleEvent->data_addr;//pt->va;
		if(true == ADDRESSES_OVERLAP(currEvent->data_addr, currEvent->accessLength, sampleEvent->data_addr, wpt->watchLen)) wpt->va = (void *)currEvent->data_addr;
                int overlapBytes = GET_OVERLAP_BYTES(sampleEvent->data_addr, wpt->watchLen, (uintptr_t)wpt->va, currEvent->accessLength);
                if (overlapBytes <= 0) {
                        std::cout << ERRMSG_BEGIN <<"WITCH: No Overlap" << ERRMSG_END << std::endl;
			removeEventContextMapEntry(sampleEvent);
			contextTree_mtx.unlock();
                        return WP_DISABLE;
                }

//		std::cout << TOOLMSG_BEGIN << "Witch: Calculated Inc of the context tree numDiffSamples: " << numDiffSamples << " overlapbytes " << overlapBytes << TOOLMSG_END << std::endl;
//		std::cout << TOOLMSG_BEGIN << "Sample access type: "<< sampleEvent->accessType << " watch access type: " << currEvent->accessType << std::endl ;
//		std::cout << TOOLMSG_BEGIN << std::hex << "sh addr2line.sh 0x"<< sampleEvent->fixedIP << " 0x" << currEvent->fixedIP << std::dec << std::endl ;
		
		if (currEvent->cpu == sampleEvent->cpu) {
                	totalUsedBytes += inc;
			totalUsedBytes_perThread += inc;
			metric_val_t metric_val;
                        metric_val.i = inc;
			leafContextIter->second->metrics[(int)USEDBYTES].increment(metric_val);
			leafContextIter->second->metrics[(int)INEFFICIENCY].metricVal.i = ((float)(leafContextIter->second->metrics[(int)DEADBYTES].metricVal.i)/(leafContextIter->second->metrics[(int)DEADBYTES].metricVal.i + leafContextIter->second->metrics[(int)USEDBYTES].metricVal.i)) * 100;

	        } else{

			Context* leafContext = addFramesToContextTree(currEvent->eventContext_agent, leafContextIter->second, false);
                        // TODO update metrics
			
			totalDeadBytes += inc;
			totalDeadBytes_perThread += inc;
			metric_val_t metric_val;
			metric_val.i = inc;
			leafContext->metrics[(int)DEADBYTES].increment(metric_val);
			leafContextIter->second->metrics[(int)DEADBYTES].increment(metric_val);
			leafContextIter->second->metrics[(int)INEFFICIENCY].metricVal.i = ((float)(leafContextIter->second->metrics[(int)DEADBYTES].metricVal.i)/(leafContextIter->second->metrics[(int)DEADBYTES].metricVal.i + leafContextIter->second->metrics[(int)USEDBYTES].metricVal.i)) * 100;

	        //if(STORE == currEvent->accessType)
//			if(currEvent->fixedIP == 0xffffffff814be584)
			{
				printCurrentStatus();
				std::cout << TOOLMSG_BEGIN << "Sample access type: "<< sampleEvent->accessType << " watch access type: " << currEvent->accessType << std::endl ;
				std::cout << TOOLMSG_BEGIN << "Sample data addr: "<< sampleEvent->data_addr << " watch data addr: " << currEvent->data_addr << std::endl;

		                std::cout << TOOLMSG_BEGIN << std::hex << "sh addr2line.sh 0x"<< sampleEvent->fixedIP << " 0x" << currEvent->fixedIP << std::dec << std::endl ;
			//	for(int i = 0; i < 4; i++)
				{
					uint64_t oldValue = ((uint64_t *)(sampleEvent->valueAtEvent))[0];
	                                uint64_t newValue = ((uint64_t *)(currEvent->valueAtEvent))[0];
					std::cout << TOOLMSG_BEGIN << "Value: old:" << std::hex << oldValue << " new:" << newValue << std::endl ;
				}
				
			}
//		        	std::cout << RESULTMSG_BEGIN << "----------------------------> WP AccessType: "<< currEvent->accessType << RESULTMSG_END << std::endl;
        	//	std::cout << "WP AccessType: "<< currEvent->accessType <<std::endl;

    		}
		watchedSampleCount++;
		watchedSampleCount_perThread++;
//		std::cout << RESULTMSG_BEGIN << "----------------------------> Total used Bytes: " << std::dec << totalUsedBytes << " total Dead Bytes: " << totalDeadBytes << " Deadwrite Ineff: " << 100*((float)( totalDeadBytes )/(float)(totalDeadBytes+totalUsedBytes)) << " sample count: " << watchedSampleCount << " pid " << getpid() << " pthread_self " << pthread_self() << " tid "<<  syscall(SYS_gettid) << RESULTMSG_END << std::endl;
	}
	else
		std::cout << ERRMSG_BEGIN <<"WITCH: Failed to find the context tree" << ERRMSG_END << std::endl;

	removeEventContextMapEntry(sampleEvent);
//	event_context_mtx.unlock(); 
	contextTree_mtx.unlock();
    	return WP_DISABLE;
}

void FalseSharing::printContextTree(int bestNContextNode)
{
        contextTree_mtx.lock();

        std::vector<std::pair<Context*, SampleNum>> elems(propAttrTable.begin(), propAttrTable.end());


//              std::sort (elems.begin(), elems.end(), propAttrTable_SortBy_MetricVal);
	//        std::sort (elems.begin(), elems.end(), propAttrTable_SortBy_Sample);
//              std::sort (elems.begin(), elems.end(), propAttrTable_SortBy_EventDistance);
//	std::sort (elems.begin(), elems.end(), propAttrTable_SortBy_MetricVal_USEDBYTES);
//	std::sort (elems.begin(), elems.end(), propAttrTable_SortBy_MetricVal_REDBYTES);
//	std::sort (elems.begin(), elems.end(), propAttrTable_SortBy_MetricVal_APPROXREDBYTES);
	std::sort (elems.begin(), elems.end(), propAttrTable_SortBy_MetricVal_DEADBYTES);
//	std::sort (elems.begin(), elems.end(), propAttrTable_SortBy_MetricVal_INEFFICIENCY);
//	std::sort (elems.begin(), elems.end(), propAttrTable_SortBy_MetricVal_SAMPLECOUNT);
//	std::sort (elems.begin(), elems.end(), propAttrTable_SortBy_MetricVal_WATCHEDCOUNT);

        std::vector<std::pair<Context*, SampleNum>>::iterator leafContextIter = elems.begin();
        int count = 0;
	MyLogger *logger = MyLogger::Instance();

        for(;leafContextIter != elems.end(); leafContextIter++, count++)
        {
                if( !leafContextIter->first || (leafContextIter->first->children.size() == 0))
                {
                        continue;
                }
                std::cout << "\n\nSampled context >> Context Local INEFFICIENCY: " << leafContextIter->first->metrics[(int)INEFFICIENCY].metricVal.i << " Global Inefficiency in this context: " << (100*((float)leafContextIter->first->metrics[(int)DEADBYTES].metricVal.i / (float)(totalDeadBytes+totalUsedBytes))) << "\% sample count " << leafContextIter->second.cur_num << " EventDistance " << leafContextIter->second.avgDistance << "\n";
		*(logger->ofsInstance) << "\n\nSampled context >> Context Local INEFFICIENCY: " << leafContextIter->first->metrics[(int)INEFFICIENCY].metricVal.i << " Global Inefficiency in this context: " << (100*((float)leafContextIter->first->metrics[(int)DEADBYTES].metricVal.i / (float)(totalDeadBytes+totalUsedBytes))) << "\% sample count " << leafContextIter->second.cur_num << " EventDistance " << leafContextIter->second.avgDistance << "\n";

                printSampledContext(leafContextIter->first, 0);
                std::cout << "Watched context << \n";
		*(logger->ofsInstance) << "Watched context << \n";
                printWatchedContext(leafContextIter->first, 0);
        }

        printCurrentStatus();
        contextTree_mtx.unlock();
//      cleanContextTree();
}

