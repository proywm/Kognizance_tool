#include <stddef.h>  // defines NULL
#include "dupSpy.hpp"
#include "watchpoint_util.h"
#include "logger.hpp"


// Global static pointer used to ensure a single instance of the class.
DupSpy* DupSpy::ds_pInstance = NULL;

thread_local uint64_t DupSpy::totalDupBytes_perThread = 0;
thread_local uint64_t DupSpy::totalApproxDupBytes_perThread = 0;
thread_local uint64_t DupSpy::totalUsedBytes_perThread = 0;
thread_local uint64_t DupSpy::watchedSampleCount_perThread = 0;

uint64_t DupSpy::totalDupBytes = 0;
uint64_t DupSpy::totalApproxDupBytes = 0;
uint64_t DupSpy::totalUsedBytes = 0;
uint64_t DupSpy::watchedSampleCount = 0;

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
  
DupSpy* DupSpy::Instance()
{
   if (!ds_pInstance)   // Only allow one instance of class to be generated.
      ds_pInstance = new DupSpy;

   return ds_pInstance;
}


//WP_TriggerAction_t DupSpy::Witch_OnWP_Callback(WP_TriggerInfo_t *wpi){
WP_TriggerAction_t DupSpy::Witch_OnWP_Callback(Event *currEvent, Event *sampleEvent, WP_TriggerInfo_t *wpt){
//	cout << RESULTMSG_BEGIN << "Within Witch_OnWP_Callback \n" << RESULTMSG_END ;
	contextTree_mtx.lock();
#if defined WITCHTOOL_DUPSPY_LOAD
	if (currEvent->accessType != LOAD && currEvent->accessType != LOAD_AND_STORE){ contextTree_mtx.unlock(); return WP_RETAIN;}
#else
	
	if (currEvent->accessType != STORE && currEvent->accessType != LOAD_AND_STORE){	 
		std::cout << ERRMSG_BEGIN << "Should never happen ------------------------------------------>>>>>>>>>>>>>>>>>>>>>>>> Error data_addr: " << currEvent->data_addr << " fixed ip: " << currEvent->fixedIP << ERRMSG_END << std::endl; 
//		removeEventContextMapEntry(sampleEvent); 
		contextTree_mtx.unlock(); 
		return WP_RETAIN;
	}
#endif
//	cout << RESULTMSG_BEGIN << "OnWP callback eventId: " << sampleEvent->eventId << RESULTMSG_END << std::endl;
	auto leafContextIter = eventToContextLeafMap.find(sampleEvent->eventId);
	if(leafContextIter != eventToContextLeafMap.end())
	{
	
//		std::cout << TOOLMSG_BEGIN << "Witch: Got leaf node of context tree " << TOOLMSG_END << std::endl;

//		Context* leafContext = addFramesToContextTree(currEvent->eventContext_agent, leafContextIter->second, false);
		// Only if the addresses do NOT overlap, do we use the sample address!
//		void *sampleAddr = sampleEvent->data_addr;//pt->va;
		std::cout << TOOLMSG_BEGIN << "currEvent->data_addr " << std::hex << currEvent->data_addr << " sampleEvent->data_addr " << std::hex << sampleEvent->data_addr << TOOLMSG_END <<std::endl;
                if(true == ADDRESSES_OVERLAP(currEvent->data_addr, currEvent->accessLength, sampleEvent->data_addr, wpt->watchLen)) wpt->va = (void *)currEvent->data_addr;
                int overlapBytes = GET_OVERLAP_BYTES(sampleEvent->data_addr, wpt->watchLen, (uintptr_t)wpt->va, currEvent->accessLength);
#if defined WITCHTOOL_DUPSPY_OVERLAP
		if (overlapBytes <= 0) {
			std::cout << ERRMSG_BEGIN <<"WITCH: Not Overlapped" << ERRMSG_END << std::endl;
#else		
    		if (overlapBytes >= 0) {
			std::cout << ERRMSG_BEGIN <<"WITCH: Overlapped" << ERRMSG_END << std::endl;
#endif
			removeEventContextMapEntry(sampleEvent);
			contextTree_mtx.unlock(); 
        		return WP_DISABLE;
    		}	


#if 0
//		std::cout << TOOLMSG_BEGIN << "Witch: Calculated Inc of the context tree numDiffSamples: " << numDiffSamples << " overlapbytes " << overlapBytes << TOOLMSG_END << std::endl;
		if(((currEvent->fixedIP == 0xffffffff814be4bf) || (currEvent->fixedIP == 0xffffffff814be48f)) && (currEvent->fixedIP == sampleEvent->fixedIP))
		{
			std::cout << TOOLMSG_BEGIN << "Sample access type: "<< sampleEvent->accessType << " watch access type: " << currEvent->accessType << std::endl ;
			std::cout << TOOLMSG_BEGIN << "sh addr2line.sh 0x"<< std::hex << sampleEvent->fixedIP << " 0x" << currEvent->fixedIP << std::endl ;
		}
#endif
#if defined WITCHTOOL_DUPSPY_OVERLAP
		int firstOffest = FIRST_OVERLAPPED_BYTE_OFFSET_IN_FIRST(currEvent->data_addr, currEvent->accessLength, sampleEvent->data_addr, wpt->watchLen);
		int secondOffest = FIRST_OVERLAPPED_BYTE_OFFSET_IN_SECOND(currEvent->data_addr, currEvent->accessLength, sampleEvent->data_addr, wpt->watchLen);
#else
		int firstOffest = 0;
                int secondOffest = 0;
#endif
		if(secondOffest < -16 || secondOffest > 16 || firstOffest > 16 || firstOffest < -16)
		{
			std::cout << ERRMSG_BEGIN <<"WITCH: InValid data address: **************************************" << "currEvent->data_addr " << std::hex << currEvent->data_addr << " currEvent->accessLength " << std::dec  << currEvent->accessLength << " sampleEvent->data_addr " << std::hex << sampleEvent->data_addr << " wpt->watchLen " << std::dec << wpt->watchLen << " firstOffest " << firstOffest << " secondOffest " << secondOffest << " overlapBytes " << overlapBytes << ERRMSG_END << std::endl;
			removeEventContextMapEntry(sampleEvent);
			contextTree_mtx.unlock(); 
                        return WP_DISABLE;
		}
		
		//printCurrentStatus();
		void *sampleStartByte = (void*)sampleEvent->data_addr + secondOffest;
		void *wptStartByte = (void*)currEvent->data_addr + firstOffest;
		    // if(!IsAddressReadable(wptStartByte)) return WP_DISABLE;

		//update proportional attibution table.    
		double myProp = 1.0;
                uint32_t threshold = 100003;//update this value from param

                int64_t numDiffSamples = GetNumDiffSamplesAndReset(leafContextIter->second, myProp, threshold, (currEvent->event_time - sampleEvent->event_time));
                int64_t inc = numDiffSamples * sampleEvent->accessLength;//wpt->sampleAccessLen;
		if (inc == 0) { std::cout << ERRMSG_BEGIN <<  "inc is 0" << ERRMSG_END << std::endl; removeEventContextMapEntry(sampleEvent); contextTree_mtx.unlock(); return WP_DISABLE;}
		    
		uint8_t redBytes = 0;
		bool isFloatOperation = currEvent->floatType == ELEM_TYPE_UNKNOWN? false: true;
		if(isFloatOperation) {
#if 0
			if(((currEvent->fixedIP == 0xffffffff814be4bf) || (currEvent->fixedIP == 0xffffffff814be48f)) && (currEvent->fixedIP == sampleEvent->fixedIP))
				cout << "isFloatOperation  secondOffest "<< secondOffest << " firstOffest  " << firstOffest << "\n" ;
#endif
			switch (currEvent->floatType) {
				case ELEM_TYPE_SINGLE: {
					if (overlapBytes < (int)sizeof(float)) {
					    goto TreatLikeInteger;
					}
					if (!IS_4_BYTE_ALIGNED(sampleStartByte)) { 
					    goto TreatLikeInteger;  
					} 
					if (!IS_4_BYTE_ALIGNED(wptStartByte)) {
					    goto TreatLikeInteger;  
					}
					//float oldValue = *(float *)(sampleEvent->valueAtEvent + secondOffest); 
//			        	float newValue = *(float *)(wptStartByte);
					float oldValue = *(float *)(sampleEvent->valueAtEvent + secondOffest);
					float newValue = *(float *)(currEvent->valueAtEvent + firstOffest);
#if 1
						cout << "oldValue "<< oldValue << " newValue  " << newValue << "\n";
#endif
					if (oldValue != newValue) {
#ifdef WITCHTOOL_DUPSPY_CONSIDER_APPROXRED
					    float rate = (oldValue - newValue) / oldValue;
					    if (rate > APPROX_RATE || rate < -APPROX_RATE) redBytes = 0;
					    else redBytes = sizeof(float);
#else
					    redBytes = 0;
#endif
					} else {
					    redBytes = sizeof(float);
					}
			    	}
				break;
				case ELEM_TYPE_DOUBLE: {
					if (overlapBytes < (int)sizeof(double)) {
				    		goto TreatLikeInteger;
					}
					if (!IS_8_BYTE_ALIGNED(sampleStartByte)) { 
				    		goto TreatLikeInteger;  
					} 
					if (!IS_8_BYTE_ALIGNED(wptStartByte)) {
				    		goto TreatLikeInteger;  
					}
					double oldValue = *(double *)(sampleEvent->valueAtEvent + secondOffest); 
					double newValue = *(double *)(currEvent->valueAtEvent + firstOffest);
#if 1
						cout << "oldValue "<< oldValue << " newValue  " << newValue << "\n";
#endif
					if (oldValue != newValue) {
#ifdef WITCHTOOL_DUPSPY_CONSIDER_APPROXRED
				    		double rate = (oldValue - newValue) / oldValue;
					    	if (rate > APPROX_RATE || rate < -APPROX_RATE) redBytes = 0;
					    	else redBytes = sizeof(double);
#else
                                        	redBytes = 0;
#endif
					} else {
				    		redBytes = sizeof(double);
					}
			    	}
				break;
			    	default: // unhandled!!
					goto TreatLikeInteger;
				break;
			}
			cout << RESULTMSG_BEGIN << "Duplicate bytes:" << redBytes <<"\n" << RESULTMSG_END ;
			if (redBytes != 0) {
#if 0
//				if(((currEvent->fixedIP == 0xffffffff814be4be) || (currEvent->fixedIP == 0xffffffff814be486)) && (currEvent->fixedIP == sampleEvent->fixedIP))
				if(((currEvent->fixedIP == 0xffffffff814be4bf) || (currEvent->fixedIP == 0xffffffff814be48f)) && (currEvent->fixedIP == sampleEvent->fixedIP))
					cout << RESULTMSG_BEGIN << "Dupundant store found \n" << RESULTMSG_END ;
#endif
				cout << RESULTMSG_BEGIN << "Dupundant store found \n" << RESULTMSG_END ;
				std::cout << TOOLMSG_BEGIN << "currEvent->data_addr " << std::hex << currEvent->data_addr << " currEvent->accessLength " << std::dec  << currEvent->accessLength << " sampleEvent->data_addr " << std::hex << sampleEvent->data_addr << " wpt->watchLen " << std::dec << wpt->watchLen << " firstOffest " << firstOffest << " secondOffest " << secondOffest << " overlapBytes " << overlapBytes << TOOLMSG_END << std::endl;

				totalApproxDupBytes += inc;
				totalApproxDupBytes_perThread += inc;

				Context* leafContext = addFramesToContextTree(currEvent->eventContext_agent, leafContextIter->second, false, REDCONTEXT);
				metric_val_t metric_val;
	                        metric_val.i = inc;
        	                leafContext->metrics[(int)APPROXREDBYTES].increment(metric_val);
//				leafContext->metrics[(int)CONTEXTTYPE].metricVal.i = (int) REDCONTEXT;
				leafContextIter->second->metrics[(int)APPROXREDBYTES].increment(metric_val);
				leafContextIter->second->metrics[(int)INEFFICIENCY].metricVal.i = ((float)(leafContextIter->second->metrics[(int)REDBYTES].metricVal.i + leafContextIter->second->metrics[(int)APPROXREDBYTES].metricVal.i)/(leafContextIter->second->metrics[(int)REDBYTES].metricVal.i + leafContextIter->second->metrics[(int)APPROXREDBYTES].metricVal.i + leafContextIter->second->metrics[(int)USEDBYTES].metricVal.i)) * 100;
				
			} else {
			    	totalUsedBytes += inc;
				totalUsedBytes_perThread += inc;
				metric_val_t metric_val;
                                metric_val.i = inc;
				leafContextIter->second->metrics[(int)USEDBYTES].increment(metric_val);
#ifdef SHOW_USED_CONTEXT
				Context* leafContext = addFramesToContextTree(currEvent->eventContext_agent, leafContextIter->second, false, USEDCONTEXT);
//				leafContext->metrics[(int)CONTEXTTYPE].metricVal.i = (int) USEDCONTEXT;
#endif

			}
		} else {
		    	TreatLikeInteger:
				for(int i = firstOffest, k = secondOffest ; i < firstOffest + sizeof(int); i++, k++) {
#if 1
						cout << "oldValue "<< ((uint8_t *)(currEvent->valueAtEvent))[i] << " newValue  " << ((uint8_t *)(sampleEvent->valueAtEvent))[k] << "\n";
#endif
			    		if(((uint8_t *)(currEvent->valueAtEvent))[i] == ((uint8_t *)(sampleEvent->valueAtEvent))[k]) {
						redBytes++;
			    		} else {
						redBytes = 0;
						break;
			    		}
				}        
				if (redBytes != 0) {
#if 0
//					if(((currEvent->fixedIP == 0xffffffff814be4be) || (currEvent->fixedIP == 0xffffffff814be486)) && (currEvent->fixedIP == sampleEvent->fixedIP))
					if(((currEvent->fixedIP == 0xffffffff814be4bf) || (currEvent->fixedIP == 0xffffffff814be48f)) && (currEvent->fixedIP == sampleEvent->fixedIP))
					{
						cout << RESULTMSG_BEGIN << "Dupundant store found \n" << RESULTMSG_END ;
						cout << "Int: oldValue: " << std::hex << ((uint64_t *)(currEvent->valueAtEvent))[0] << " newValue:  " << ((uint64_t *)(sampleEvent->valueAtEvent))[0] << std::dec <<"\n";
					}
#endif
					cout << RESULTMSG_BEGIN << "Duplicate store found \n" << RESULTMSG_END ;
					std::cout << TOOLMSG_BEGIN << "currEvent->data_addr " << std::hex << currEvent->data_addr << " currEvent->accessLength " << std::dec  << currEvent->accessLength << " sampleEvent->data_addr " << std::hex << sampleEvent->data_addr << " wpt->watchLen " << std::dec << wpt->watchLen << " firstOffest " << firstOffest << " secondOffest " << secondOffest << " overlapBytes " << overlapBytes << " currEvent->fixedIP: " << std::hex << currEvent->fixedIP << " sampleEvent->fixedIP: " << std::hex << sampleEvent->fixedIP << TOOLMSG_END << std::endl;

			    		totalDupBytes += inc;
					totalDupBytes_perThread += inc;

					Context* leafContext = addFramesToContextTree(currEvent->eventContext_agent, leafContextIter->second, false, REDCONTEXT);
					metric_val_t metric_val;
		                        metric_val.i = inc;
                		        leafContext->metrics[(int)REDBYTES].increment(metric_val);
//					leafContext->metrics[(int)CONTEXTTYPE].metricVal.i = (int)REDCONTEXT;
					leafContextIter->second->metrics[(int)REDBYTES].increment(metric_val);
					leafContextIter->second->metrics[(int)INEFFICIENCY].metricVal.i = ((float)(leafContextIter->second->metrics[(int)REDBYTES].metricVal.i + leafContextIter->second->metrics[(int)APPROXREDBYTES].metricVal.i)/(leafContextIter->second->metrics[(int)REDBYTES].metricVal.i + leafContextIter->second->metrics[(int)APPROXREDBYTES].metricVal.i + leafContextIter->second->metrics[(int)USEDBYTES].metricVal.i)) * 100;
					
				} else {
			    		totalUsedBytes += inc;
					totalUsedBytes_perThread += inc;
					metric_val_t metric_val;
        	                        metric_val.i = inc;
	                                leafContextIter->second->metrics[(int)USEDBYTES].increment(metric_val);
					leafContextIter->second->metrics[(int)INEFFICIENCY].metricVal.i = ((float)(leafContextIter->second->metrics[(int)REDBYTES].metricVal.i + leafContextIter->second->metrics[(int)APPROXREDBYTES].metricVal.i)/(leafContextIter->second->metrics[(int)REDBYTES].metricVal.i + leafContextIter->second->metrics[(int)APPROXREDBYTES].metricVal.i + leafContextIter->second->metrics[(int)USEDBYTES].metricVal.i)) * 100;
#ifdef SHOW_USED_CONTEXT
					Context* leafContext = addFramesToContextTree(currEvent->eventContext_agent, leafContextIter->second, false, USEDCONTEXT);
//	                                leafContext->metrics[(int)CONTEXTTYPE].metricVal.i = (int)USEDCONTEXT;
#endif
				}
		}
		watchedSampleCount_perThread++;
		watchedSampleCount++;
		std::cout << "redBytes: "<< redBytes <<  std::endl;
#if 0
//		if(((currEvent->fixedIP == 0xffffffff814be4be) || (currEvent->fixedIP == 0xffffffff814be486)) && (currEvent->fixedIP == sampleEvent->fixedIP))
		if(((currEvent->fixedIP == 0xffffffff814be4bf) || (currEvent->fixedIP == 0xffffffff814be48f)) && (currEvent->fixedIP == sampleEvent->fixedIP))
			printCurrentStatus();
#endif
    	}
	else
		std::cout << ERRMSG_BEGIN <<"WITCH: Failed to find the context tree : sampleEvent->eventId " << sampleEvent->eventId << ERRMSG_END << std::endl;

	removeEventContextMapEntry(sampleEvent);
	contextTree_mtx.unlock(); 
	std::cout << "BP catched @ fixed IP: " << std::hex << currEvent->fixedIP  << std::endl;
    	return WP_DISABLE;
}

void DupSpy::printContextTree(int bestNContextNode)
{
        contextTree_mtx.lock();

#if 1
	MyLogger *logger = MyLogger::Instance();
	calculateForAllContext(&contextTreeRoot, 0);
	unsigned int totalBytes = totalDupBytes + totalApproxDupBytes + totalUsedBytes;
	printAllContext(&contextTreeRoot, 0, totalBytes);
	logger->close();
#else

        std::vector<std::pair<Context*, SampleNum>> elems(propAttrTable.begin(), propAttrTable.end());


//              std::sort (elems.begin(), elems.end(), propAttrTable_SortBy_MetricVal);
//        std::sort (elems.begin(), elems.end(), propAttrTable_SortBy_Sample);
//              std::sort (elems.begin(), elems.end(), propAttrTable_SortBy_EventDistance);

//std::sort (elems.begin(), elems.end(), propAttrTable_SortBy_MetricVal_USEDBYTES);
std::sort (elems.begin(), elems.end(), propAttrTable_SortBy_MetricVal_REDBYTES);
//std::sort (elems.begin(), elems.end(), propAttrTable_SortBy_MetricVal_APPROXREDBYTES);
//std::sort (elems.begin(), elems.end(), propAttrTable_SortBy_MetricVal_DEADBYTES);
//std::sort (elems.begin(), elems.end(), propAttrTable_SortBy_MetricVal_SAMPLECOUNT);
//std::sort (elems.begin(), elems.end(), propAttrTable_SortBy_MetricVal_WATCHEDCOUNT);
//	std::sort (elems.begin(), elems.end(), propAttrTable_SortBy_MetricVal_INEFFICIENCY);

        std::vector<std::pair<Context*, SampleNum>>::iterator leafContextIter = elems.begin();
        int count = 0;
	
	MyLogger *logger = MyLogger::Instance();


        for(;leafContextIter != elems.end(); leafContextIter++, count++)
        {
                if( !leafContextIter->first || (leafContextIter->first->children.size() == 0))
                {
                        continue;
                }
                //std::cout << "Sampled context >> metricVal: " << leafContextIter->first->metric.metricVal.i << " Inefficiency in this context: " << (100*((float)leafContextIter->first->metric.metricVal.i / (float)(totalDupBytes+totalApproxDupBytes+totalUsedBytes))) << "\% sample count " << leafContextIter->second.cur_num << " EventDistance " << leafContextIter->second.avgDistance << "\n";

		std::cout << "Sampled context >> Local INEFFICIENCY: " << leafContextIter->first->metrics[(int)INEFFICIENCY].metricVal.i << " Inefficiency in this context: " << (100*((float)(leafContextIter->first->metrics[(int)REDBYTES].metricVal.i + leafContextIter->first->metrics[(int)APPROXREDBYTES].metricVal.i)/ (float)(totalDupBytes + totalApproxDupBytes + totalUsedBytes))) << "\% sample count " << leafContextIter->second.cur_num << " EventDistance " << leafContextIter->second.avgDistance << "\n";

		*(logger->ofsInstance) << "Sampled context >> Local INEFFICIENCY: " << leafContextIter->first->metrics[(int)INEFFICIENCY].metricVal.i << " Inefficiency in this context: " << (100*((float)(leafContextIter->first->metrics[(int)REDBYTES].metricVal.i + leafContextIter->first->metrics[(int)APPROXREDBYTES].metricVal.i)/ (float)(totalDupBytes + totalApproxDupBytes + totalUsedBytes))) << "\% sample count " << leafContextIter->second.cur_num << " EventDistance " << leafContextIter->second.avgDistance << "\n";

                printSampledContext(leafContextIter->first, 0);
                std::cout << "Watched context << \n";

		*(logger->ofsInstance) << "Watched context << \n";
                printWatchedContext(leafContextIter->first, 0);
        }

        printCurrentStatus();
	logger->close();
#endif

        contextTree_mtx.unlock();

//      cleanContextTree();
}

