//
// debug register interface
//
#ifndef _WATCHPOINT_H
#define _WATCHPOINT_H

#include <stdint.h>
#include <signal.h>
#include "context.hpp"

/* ================================================================== */
// Debug register data types and structures
/* ================================================================== */
typedef enum WP_Access_t {WP_READ, WP_WRITE, WP_RW, WP_INST, WP_INVALID} WP_Access_t;
typedef enum WP_ReplacementPolicy_t {WP_REPLACEMENT_AUTO, WP_REPLACEMENT_EMPTY_SLOT, WP_REPLACEMENT_OLDEST, WP_REPLACEMENT_NEWEST} WP_ReplacementPolicy_t;
//typedef enum WP_MergePolicy_t {AUTO_MERGE, NO_MERGE, CLIENT_ACTION} MergePolicy_t;
//typedef enum OverwritePolicy {OVERWRITE, NO_OVERWRITE} OverwritePolicy;
//typedef enum VictimType {EMPTY_SLOT, NON_EMPTY_SLOT, NONE_AVAILABLE} VictimType;
typedef enum WP_TriggerAction_t {WP_DISABLE, WP_ALREADY_DISABLED, WP_DISABLE_ALL, WP_RETAIN} WP_TriggerAction_t;

// Data structure that is captured when a WP triggers
#if 0
typedef struct WP_TriggerInfo_t{
    void * va;
    int watchLength;
    WP_Access_t trappedAccessType;
    void * ctxt;
    void * pc;
    int pcPrecise; // 1: precise; 0: not precise
    void * data; //user defined
} WP_TriggerInfo_t;
#endif

// Data structure that is captured when a WP triggers
typedef struct WP_TriggerInfo_t{
    void *va;
    int watchLen;
    void *watchCtxt;
    WP_Access_t trappedAccessType;
    void *uCtxt;
    void *pc;
    int pcPrecise; // 1: precise; 0: not precise
    int sampleAccessLen;
    void *sampleValue;
    int metricID;
} WP_TriggerInfo_t;

typedef WP_TriggerAction_t (*WP_TrapCallback_t)( WP_TriggerInfo_t *);
typedef void (*WP_PerfCallback_t) ();

#ifdef __cplusplus
extern "C"
{
#endif

//*****************************Basics*****************************//
extern int WP_Init();
extern void WP_Shutdown();
extern int WP_ThreadInit(WP_TrapCallback_t cb_func);
extern void WP_ThreadTerminate();
extern int WP_Subscribe(void * va, int watch_length, int accessLen, WP_Access_t watch_type, Event *sampleEvent);
extern void OnWatchPoint(int fd, void *context, Event* event);

//*****************************Advanced Configuration********************//

//If you are using perf events in your own system, you may want to pause all your perf events just after a watchpoint is captured and resume all your perf events just before finishing handling a trap.
extern void WP_SetPerfPauseAndResumeFunctions(WP_PerfCallback_t pause_fn,  WP_PerfCallback_t resume_fn);



#ifdef __cplusplus
}
#endif


#endif /* _WATCHPOINT_H */
