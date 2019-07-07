#ifndef _DUPSPY_H_
#define _DUPSPY_H_

#include "WitchLogic.hpp"

using namespace std;

class DupSpy : public WitchLogic
{
private:
        DupSpy(){};
        DupSpy(DupSpy const&){};
        WitchLogic& operator=(WitchLogic const&){};
        static DupSpy* ds_pInstance;
	static thread_local uint64_t totalDupBytes_perThread;
	static thread_local uint64_t totalApproxDupBytes_perThread;
	static thread_local uint64_t totalUsedBytes_perThread;
	static thread_local uint64_t watchedSampleCount_perThread;

	static uint64_t totalDupBytes;
	static uint64_t totalApproxDupBytes;
	static uint64_t totalUsedBytes;
	static uint64_t watchedSampleCount;

public:
        static DupSpy* Instance();

        WP_TriggerAction_t Witch_OnWP_Callback(Event *currEvent, Event *sampleEvent, WP_TriggerInfo_t *wpt);

	void resetWitchLogic(void) { totalDupBytes = totalApproxDupBytes = totalUsedBytes = watchedSampleCount = 0;}
	
	void printCurrentStatus(void) {
		std::cout << RESULTMSG_BEGIN << "----------------------------> Total used Bytes: " << std::dec << totalUsedBytes << " total Dup Bytes: " << totalDupBytes << " totalApproxDupBytes: " << totalApproxDupBytes << " Ineff: " << 100 * ((float)( totalDupBytes + totalApproxDupBytes)/(float)(totalDupBytes+totalApproxDupBytes+totalUsedBytes)) << "\% sample count: " << watchedSampleCount  << RESULTMSG_END << std::endl;
        }

	void printContextTree(int bestNContextNode);

};




#endif
