#ifndef _WITCHLOGIC_H_
#define _WITCHLOGIC_H_

#include  <algorithm>
#include <mutex>

#include "platform.hpp"
#include "tool.hpp"
#include "context.hpp"

#include "watchpoint.h"

using namespace std;

typedef struct SampleNum {
    uint64_t catchup_num;
    uint64_t cur_num;
    double avgDistance;
} SampleNum_t;

typedef enum MetricType {USEDBYTES, REDBYTES, APPROXREDBYTES, DEADBYTES, SAMPLECOUNT, WATCHEDCOUNT, INEFFICIENCY, CONTEXTTYPE} MetricType;
typedef enum ContextType {SAMPLEDCONTEXT, USEDCONTEXT, DEADCONTEXT, REDCONTEXT, UNDEFINEDCONTEXT} ContextType;

#if 0
struct propAttrTablecomp {
  bool operator() (const std::pair<Context*, SampleNum>& lhs, const std::pair<Context*, SampleNum>& rhs) const
  {return lhs.first->metric.metricVal < rhs.first->metric.metricVal;}
}propAttrTablecompObj;
#else
//bool propAttrTablecompObj(const std::pair<Context*, SampleNum>& lhs, const std::pair<Context*, SampleNum>& rhs){ return lhs.first->metric.metricVal < rhs.first->metric.metricVal;}
//bool propAttrTablecompObj(const std::pair<Context*, SampleNum>& lhs, const std::pair<Context*, SampleNum>& rhs){ return lhs.second.catchup_num < rhs.second.catchup_num;}
#endif

struct propAttrTablecomp_By_MetricVal_USEDBYTES {
  bool operator() (const std::pair<Context*, SampleNum>& lhs, const std::pair<Context*, SampleNum>& rhs) const
  {return lhs.first->metrics[(int)USEDBYTES].metricVal < rhs.first->metrics[(int)USEDBYTES].metricVal;}
};

struct propAttrTablecomp_By_MetricVal_REDBYTES {
  bool operator() (const std::pair<Context*, SampleNum>& lhs, const std::pair<Context*, SampleNum>& rhs) const
  {return lhs.first->metrics[(int)REDBYTES].metricVal < rhs.first->metrics[(int)REDBYTES].metricVal;}
};

struct propAttrTablecomp_By_MetricVal_APPROXREDBYTES {
  bool operator() (const std::pair<Context*, SampleNum>& lhs, const std::pair<Context*, SampleNum>& rhs) const
  {return lhs.first->metrics[(int)APPROXREDBYTES].metricVal < rhs.first->metrics[(int)APPROXREDBYTES].metricVal;}
};

struct propAttrTablecomp_By_MetricVal_DEADBYTES {
  bool operator() (const std::pair<Context*, SampleNum>& lhs, const std::pair<Context*, SampleNum>& rhs) const
  {return lhs.first->metrics[(int)DEADBYTES].metricVal < rhs.first->metrics[(int)DEADBYTES].metricVal;}
};

struct propAttrTablecomp_By_MetricVal_SAMPLECOUNT {
  bool operator() (const std::pair<Context*, SampleNum>& lhs, const std::pair<Context*, SampleNum>& rhs) const
  {return lhs.first->metrics[(int)SAMPLECOUNT].metricVal < rhs.first->metrics[(int)SAMPLECOUNT].metricVal;}
};

struct propAttrTablecomp_By_MetricVal_WATCHEDCOUNT {
  bool operator() (const std::pair<Context*, SampleNum>& lhs, const std::pair<Context*, SampleNum>& rhs) const
  {return lhs.first->metrics[(int)WATCHEDCOUNT].metricVal < rhs.first->metrics[(int)WATCHEDCOUNT].metricVal;}
};

struct propAttrTablecomp_By_MetricVal_INEFFICIENCY {
  bool operator() (const std::pair<Context*, SampleNum>& lhs, const std::pair<Context*, SampleNum>& rhs) const
  {return lhs.first->metrics[(int)INEFFICIENCY].metricVal < rhs.first->metrics[(int)INEFFICIENCY].metricVal;}
};

//propAttrTable_SortBy_MetricVal;

struct propAttrTablecomp_By_Sample {
  bool operator() (const std::pair<Context*, SampleNum>& lhs, const std::pair<Context*, SampleNum>& rhs) const
  {return lhs.second.catchup_num < rhs.second.catchup_num;}
}; 

//propAttrTable_SortBy_Sample;

struct propAttrTablecomp_By_EventDistance {
  bool operator() (const std::pair<Context*, SampleNum>& lhs, const std::pair<Context*, SampleNum>& rhs) const
  {return lhs.second.avgDistance > rhs.second.avgDistance;}
};

//propAttrTable_SortBy_EventDistance;

class WitchLogic
{
private:
	//returns the leaf frame pointer
	Context* addFramesToContextTree( std::vector<ContextFrame*> contextFrameList, Context* root, bool direction_ParentToChild, ContextType ctype = SAMPLEDCONTEXT)
	{
		int i = 0;
//		contextTree_mtx.lock();
		std::vector<ContextFrame*>::iterator iter;

		Context* currentNode = root;//&contextTreeRoot;
		
		if(!direction_ParentToChild)
		{
			iter = contextFrameList.begin();
		}
		else
		{
			iter = contextFrameList.end() - 1;
		}
		std::cout << "context tree length: " << contextFrameList.size() << std::endl;
		while(i < contextFrameList.size())
		{
//			 std::cout << "depth: "<< i << " " << (*iter)->source_file <<":"<< (*iter)->src_lineno << ", method: "<< (*iter)->method_name << "(), Code Object Address: " << (*iter)->binary_addr <<std::endl;
			 Context *newContext = new Context(*iter);
			 if(currentNode != nullptr)
			 {
				auto nodeIter = currentNode->children.find(newContext);
				if(nodeIter != currentNode->children.end())
				{
					delete newContext;
				}
				else
				{
					newContext->_parent = currentNode;
					currentNode->children.insert(newContext);
					nodeIter = currentNode->children.find(newContext);
				}
				currentNode = *(nodeIter);
				if(currentNode->metrics[(int)CONTEXTTYPE].metricVal.i != SAMPLEDCONTEXT && currentNode->metrics[(int)CONTEXTTYPE].metricVal.i != ctype)
					currentNode->metrics[(int)CONTEXTTYPE].metricVal.i = (int) UNDEFINEDCONTEXT;
				currentNode->metrics[(int)CONTEXTTYPE].metricVal.i = (int) ctype;
			 }
			 i++;
			 if(!direction_ParentToChild)
				iter++;
			 else
				iter--;
		}
//		contextTree_mtx.unlock();
		return currentNode;
	}
public:
	//maintain a calling context tree
	Context contextTreeRoot;
	std::mutex contextTree_mtx;
	std::map<int, Context*> eventToContextLeafMap;
	std::unordered_map<Context*, SampleNum> propAttrTable;
	std::mutex event_context_mtx;

	WitchLogic(){ };
	~WitchLogic(){};

	bool Witch_OnSample_Callback(Event *event){
		contextTree_mtx.lock();
		Context* leafContext = addFramesToContextTree(event->eventContext_agent, &contextTreeRoot, true);
		if(leafContext != &contextTreeRoot)
		{
//			event_context_mtx.lock();

//			sampleEventList.push_back(event);
			eventToContextLeafMap[event->eventId] = leafContext;
			if(propAttrTable.find(leafContext) != propAttrTable.end())
				propAttrTable[leafContext].cur_num++;
			else
			{
				SampleNum counters;
				counters.cur_num = 1;
				counters.catchup_num = 0;
				counters.avgDistance = 0;
				propAttrTable.insert(std::pair<Context*, SampleNum>(leafContext, counters));
//				propAttrTable[leafContext].cur_num = 1;
//				propAttrTable[leafContext].catchup_num = 0;
			}

//			event_context_mtx.unlock();
//			std::cout << "----------------------------------------> PEBS leaf method : " << leafContext->getContextFrame_methodName() << std::endl;
		}
		else
		{
			std::cout << "----------------------------------------> PEBS failed for event : " << event->eventId << std::endl;
			contextTree_mtx.unlock();
			return false;
		}
//		temp++;
//		std::cout << "----------------------------------------> PEBS sample, propAttrTable.size(): "<< std::dec << propAttrTable.size() << " temp " << temp <<std::endl;
		contextTree_mtx.unlock();
		return true;
	}
	double GetNumDiffSamplesAndReset(Context* leafContext, double prop, uint32_t threshold, uint64_t dist)
	{
//		event_context_mtx.lock();
		if(propAttrTable.find(leafContext) == propAttrTable.end())
		{
			std::cout << ERRMSG_BEGIN << "Witch: Error could not find propattribute entry in table. method: " << leafContext->getContextFrame_methodName() << ERRMSG_END << std::endl;				    
			//event_context_mtx.unlock();
			return 0;
		}
//		std::cout << "----------------------------------------> BP leaf method : " << leafContext->getContextFrame_methodName() << std::endl;
		double diff = 0., diffWithPeriod = 0.;
                diff = (propAttrTable[leafContext].cur_num - propAttrTable[leafContext].catchup_num) * prop;
//		std::cout << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> GetNumDiffSamplesAndReset cur_num: " << propAttrTable[leafContext].cur_num << "cactchup_num: " << propAttrTable[leafContext].catchup_num << std::endl;
                diffWithPeriod = diff * threshold;

                propAttrTable[leafContext].catchup_num = propAttrTable[leafContext].cur_num;
		//calculate running avg
		propAttrTable[leafContext].avgDistance = ((double)( propAttrTable[leafContext].avgDistance + (double)dist)/propAttrTable[leafContext].catchup_num);
//		std::cout << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> .avgDistance: " << propAttrTable[leafContext].avgDistance << std::endl;
		//event_context_mtx.unlock();
		return diffWithPeriod;
	}
	void printSampledContext(Context* root, int i)
	{

		if(root != nullptr)
		{
#ifdef AVOID_SEEN
			if( i==0 && inAlreadySeenList(root->frame.binary_addr))
				return;
#endif
			MyLogger *logger = MyLogger::Instance();
			int k = 0;
                        while(k != i)
                        {
                                std::cout << " ";
                                *(logger->ofsInstance) << " ";
                         	k++;
                        }
			std::cout << "|_ depth: "<< i << " " << root->frame.source_file <<":" << std::hex << root->frame.src_lineno << ", method: "<< root->frame.method_name << "(), Code Object Address: " << root->frame.binary_addr << std::dec <<std::endl;
			*(logger->ofsInstance) << "|_ depth: "<< i << " " << root->frame.source_file <<":" << std::hex << root->frame.src_lineno << ", method: "<< root->frame.method_name << "(), Code Object Address: " << root->frame.binary_addr << std::dec <<std::endl;
			printSampledContext(root->_parent, i+1);
		}
	}
	void printWatchedContext(Context* root, int i)
        {
                if(root != nullptr)
                {

#ifdef AVOID_SEEN
			if( i==0 && inAlreadySeenList(root->frame.binary_addr))
                                return;

#endif

			MyLogger *logger = MyLogger::Instance();
			if(i != 0)
			{
				int k = 0;
				while(k != i)
				{
					std::cout << " ";
					*(logger->ofsInstance) << " ";
					k++;
				}
				std::cout << "|_ depth: "<< i << " " << root->frame.source_file <<":" << std::hex << root->frame.src_lineno << ", method: "<< root->frame.method_name << "(), Code Object Address: " << root->frame.binary_addr << std::dec ;//<<std::endl;
				*(logger->ofsInstance) << "|_ depth: "<< i << " " << root->frame.source_file <<":" << std::hex << root->frame.src_lineno << ", method: "<< root->frame.method_name << "(), Code Object Address: " << root->frame.binary_addr << std::dec ;//<<std::endl;


				//print context type
				if(root->metrics[(int)CONTEXTTYPE].metricVal.i == (int)USEDCONTEXT)
				{
					std::cout << "[U]" <<std::endl;
					*(logger->ofsInstance) << "[U]" <<std::endl;
				}
				else if(root->metrics[(int)CONTEXTTYPE].metricVal.i == (int)DEADCONTEXT)
				{
					std::cout << "[D]" <<std::endl;
                                        *(logger->ofsInstance) << "[D]" <<std::endl;
				}
				else if(root->metrics[(int)CONTEXTTYPE].metricVal.i == (int)REDCONTEXT)
				{
					std::cout << "[R]" <<std::endl;
					*(logger->ofsInstance) << "[R]" <<std::endl;
				}
				else if (root->metrics[(int)CONTEXTTYPE].metricVal.i == (int)UNDEFINEDCONTEXT)
				{
					std::cout << "[UD]" <<std::endl;
                                        *(logger->ofsInstance) << "[UD]" <<std::endl;
				}
				else
				{
					std::cout <<std::endl;
                                        *(logger->ofsInstance) <<std::endl;
				}

			}
			auto nodeIter = root->children.begin();
			for(;nodeIter != root->children.end(); nodeIter++)
			{
#ifdef SHOW_SAMPLED_WATCHED_SAME_ADDRESS
				if(i == 0 && *nodeIter != nullptr && (root->frame.binary_addr == (*nodeIter)->frame.binary_addr))
				{
					std::cout << " << *** >> " << std::endl;
					*(logger->ofsInstance) << " << *** >> " << std::endl;
				}
#endif
//				if(*nodeIter == nullptr)
//					std::cout << "<<< " << root->metric.metricVal.i ;
				printWatchedContext(*nodeIter, i+1);
			}
                }
        }
	void printAllContext(Context* root, int i, unsigned int totalBytes)
	{
		MyLogger *logger = MyLogger::Instance();
		if(root != nullptr)
                {
			if(root->_parent && root->frame.binary_addr == root->_parent->frame.binary_addr)
                                return ;
                        auto nodeIter = root->children.begin();
			if(!(root->metrics[(int)USEDBYTES].metricVal.i==0 && root->metrics[(int)REDBYTES].metricVal.i == 0))
                        {
				int k = 0; 
	                        while(k != i)
        	                {
                	        	std::cout << " ";
                        	        *(logger->ofsInstance) << " ";
                                	k++;
	                        }
//				float inEff = 100 * ((float)(root->metrics[(int)REDBYTES].metricVal.i+root->metrics[(int)USEDBYTES].metricVal.i)) / (float)totalBytes;
				double avgDist = 1000000;
				double inEff = 100 * ((double)(root->metrics[(int)REDBYTES].metricVal.i)) / totalBytes;
                                if(inEff > 1.0 ) std::cout << OUTMSG_BEGIN;

				if(propAttrTable.find(root) != propAttrTable.end())
	                                avgDist = propAttrTable[root].avgDistance;
				
                        	std::cout << "|_ depth: "<< i << " " << root->frame.source_file <<":" << std::hex << root->frame.src_lineno << ", method: "<< root->frame.method_name << "(), Code Object Address: " << root->frame.binary_addr << std::dec << " redBytes: " << root->metrics[(int)REDBYTES].metricVal.i << " usedBytes: " << root->metrics[(int)USEDBYTES].metricVal.i << " avgDst: " << avgDist << " InEff %: " << inEff << std::endl;
                        	*(logger->ofsInstance) << "|_ depth: "<< i << " " << root->frame.source_file <<":" << std::hex << root->frame.src_lineno << ", method: "<< root->frame.method_name << "(), Code Object Address: " << root->frame.binary_addr << std::dec << " redBytes: " << root->metrics[(int)REDBYTES].metricVal.i << " usedBytes: " << root->metrics[(int)USEDBYTES].metricVal.i << " avgDst: " << avgDist << " InEff %: " << inEff << std::endl;

				if(inEff > 1.0) std::cout << OUTMSG_END;
					
			
			}

			
                        for(;nodeIter != root->children.end(); nodeIter++)
                        {
                                printAllContext(*nodeIter, i+1, totalBytes);
                        }
                }

	}
	metric_val_t calculateForAllContext(Context* root, int i)
        {
                MyLogger *logger = MyLogger::Instance();
		metric_val_t metric_val_red;
                if(root != nullptr)
                {
			if(root->_parent && root->frame.binary_addr == root->_parent->frame.binary_addr)
				return metric_val_red;
                        auto nodeIter = root->children.begin();
			
                        for(;nodeIter != root->children.end(); nodeIter++)
                        {
//                                std::cout << "|_ depth: "<< i << " " << root->frame.source_file <<":" << std::hex << root->frame.src_lineno << ", method: "<< root->frame.method_name << "(), Code Object Address: " << root->frame.binary_addr << std::dec <<std::endl;
 //                               *(logger->ofsInstance) << "|_ depth: "<< i << " " << root->frame.source_file <<":" << std::hex << root->frame.src_lineno << ", method: "<< root->frame.method_name << "(), Code Object Address: " << root->frame.binary_addr << std::dec << std::endl;
                                metric_val_t ret = calculateForAllContext(*nodeIter, i+1);
				metric_val_red.i += ret.i;
				metric_val_red.r += ret.r;
                        }
			root->metrics[(int)REDBYTES].metricVal.i += metric_val_red.i;
			root->metrics[(int)USEDBYTES].metricVal.i += metric_val_red.r;

			metric_val_red.i = root->metrics[(int)REDBYTES].metricVal.i;
			metric_val_red.r = root->metrics[(int)USEDBYTES].metricVal.i;
                }
		return metric_val_red;

        }

        void cleanChildContext(Context* root, int i)
        {
                if(root != nullptr)
                {
                        auto nodeIter = root->children.begin();
                        for(;nodeIter != root->children.end(); nodeIter++)
                        {
                                cleanChildContext(*nodeIter, i+1);
                        }
			if(i !=0 )
                                delete root;
                }
        }
	void cleanContextTree();
	//called with event_context_mtx.lock();
	void removeEventContextMapEntry(Event *event)
	{
		if(event != nullptr)
			eventToContextLeafMap.erase(event->eventId);
	}

	bool inAlreadySeenList(unsigned long ipAddr);
	
	virtual void printContextTree(int bestNContextNode) = 0;
	virtual WP_TriggerAction_t Witch_OnWP_Callback(Event *currEvent, Event *sampleEvent, WP_TriggerInfo_t *wpt)=0;
	virtual void resetWitchLogic(void)=0;
	virtual void printCurrentStatus(void)=0;

	//friend classes
        friend class DeadStore;
	friend class RedSpy;
	friend class LoadSpy;
	friend class DupSpy;
	friend class FalseSharing;
};




#endif
