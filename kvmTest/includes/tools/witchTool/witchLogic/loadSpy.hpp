#ifndef _LOADSPY_H_
#define _LOADSPY_H_

#include "WitchLogic.hpp"

using namespace std;

class LoadSpy : public WitchLogic
{
private:
        LoadSpy(){};
        LoadSpy(LoadSpy const&){};
        WitchLogic& operator=(WitchLogic const&){};
        static LoadSpy* rs_pInstance;
	static thread_local uint64_t totalRedBytes_perThread;
	static thread_local uint64_t totalApproxRedBytes_perThread;
	static thread_local uint64_t totalUsedBytes_perThread;
	static thread_local uint64_t watchedSampleCount_perThread;

	static uint64_t totalRedBytes;
        static uint64_t totalApproxRedBytes;
        static uint64_t totalUsedBytes;
        static uint64_t watchedSampleCount;


public:
        static LoadSpy* Instance();

        WP_TriggerAction_t Witch_OnWP_Callback(Event *currEvent, Event *sampleEvent, WP_TriggerInfo_t *wpt);
	void resetWitchLogic(void) { totalRedBytes = totalApproxRedBytes = totalUsedBytes = watchedSampleCount = 0;}

	void printCurrentStatus(void) {
                std::cout << RESULTMSG_BEGIN << "----------------------------> Total used Bytes: " << std::dec << totalUsedBytes << " total Red Bytes: " << totalRedBytes << " totalApproxRedBytes: " << totalApproxRedBytes << " Ineff: " << 100 * ((float)( totalRedBytes + totalApproxRedBytes)/(float)(totalRedBytes+totalApproxRedBytes+totalUsedBytes)) << "\% sample count: " << watchedSampleCount  << RESULTMSG_END << std::endl;
        }
	void printContextTree(int bestNContextNode);

};




#endif
