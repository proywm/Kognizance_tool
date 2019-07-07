#ifndef _FALSESHARE_H_
#define _FALSESHARE_H_

#include "WitchLogic.hpp"

using namespace std;

class FalseSharing : public WitchLogic
{
private:
        FalseSharing(){};
        FalseSharing(FalseSharing const&){};
        WitchLogic& operator=(WitchLogic const&){};
        static FalseSharing* ds_pInstance;
	
	static thread_local uint64_t totalDeadBytes_perThread;
	static thread_local uint64_t totalUsedBytes_perThread;
	static thread_local uint64_t watchedSampleCount_perThread;

	static uint64_t totalDeadBytes;
        static uint64_t totalUsedBytes;
        static uint64_t watchedSampleCount;

public:
        static FalseSharing* Instance();

        WP_TriggerAction_t Witch_OnWP_Callback(Event *currEvent, Event *sampleEvent, WP_TriggerInfo_t *wpt);

	void resetWitchLogic(void) {	totalDeadBytes = totalUsedBytes = watchedSampleCount = 0; }

	void printCurrentStatus(void) { 
		std::cout << RESULTMSG_BEGIN << "----------------------------> Total true sharing Bytes: " << std::dec << totalUsedBytes << " total false sharing Bytes: " << totalDeadBytes << " False Share Ineff: " << 100*((float)( totalDeadBytes )/(float)(totalDeadBytes+totalUsedBytes)) << " sample count: " << watchedSampleCount <<  RESULTMSG_END << std::endl;
	}
	void printContextTree(int bestNContextNode);
};




#endif
