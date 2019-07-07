#include <stddef.h>  // defines NULL
#include "WitchLogic.hpp"

unsigned long SeenList[] = {0xffffffff810c3ac2, 0xffffffff810bea30, 0xffffffff810cae83, 0xffffffff810cb70b, 0xffffffff811145f5, 0xffffffff819f051f, 0xffffffff819f08d9, 0xffffffff81398ce6};
//struct propAttrTablecomp_By_MetricVal propAttrTable_SortBy_MetricVal;
struct propAttrTablecomp_By_MetricVal_USEDBYTES propAttrTable_SortBy_MetricVal_USEDBYTES;
struct propAttrTablecomp_By_MetricVal_REDBYTES propAttrTable_SortBy_MetricVal_REDBYTES;
struct propAttrTablecomp_By_MetricVal_APPROXREDBYTES propAttrTable_SortBy_MetricVal_APPROXREDBYTES;
struct propAttrTablecomp_By_MetricVal_DEADBYTES propAttrTable_SortBy_MetricVal_DEADBYTES;
struct propAttrTablecomp_By_MetricVal_SAMPLECOUNT propAttrTable_SortBy_MetricVal_SAMPLECOUNT;
struct propAttrTablecomp_By_MetricVal_WATCHEDCOUNT propAttrTable_SortBy_MetricVal_WATCHEDCOUNT;
struct propAttrTablecomp_By_MetricVal_INEFFICIENCY propAttrTable_SortBy_MetricVal_INEFFICIENCY;

struct propAttrTablecomp_By_Sample propAttrTable_SortBy_Sample;
struct propAttrTablecomp_By_EventDistance propAttrTable_SortBy_EventDistance;

void WitchLogic::cleanContextTree()
{
	std::cout << "from cleanChildContext \n";
	contextTree_mtx.lock();

	propAttrTable.erase(propAttrTable.begin(), propAttrTable.end());
	eventToContextLeafMap.erase(eventToContextLeafMap.begin(), eventToContextLeafMap.end());
	cleanChildContext(&contextTreeRoot, 0);

//	propAttrTable.erase(propAttrTable.begin(), propAttrTable.end());
//	eventToContextLeafMap.erase(eventToContextLeafMap.begin(), eventToContextLeafMap.end());

	contextTree_mtx.unlock();
	resetWitchLogic();
}

bool WitchLogic::inAlreadySeenList(unsigned long ipAddr)
{
	for(int iter = 0; iter < (sizeof(SeenList)/sizeof(unsigned long)); iter++)
        {
        	if(SeenList[iter] == ipAddr)
                	return true;
        }
        return false;
}
