#ifndef __WATCHPOINT_UTIL_H
#define __WATCHPOINT_UTIL_H
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <linux/perf_event.h>
#include <linux/hw_breakpoint.h>
#include <string.h>
#include <time.h>
#include <linux/kernel.h>
#include <unistd.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>

#include <assert.h>
#include <asm/unistd.h>
#include <stdio.h>
#include <sys/mman.h>

#include "context.hpp"

//********************MACRO*************************************/
#define MAX_WP_LENGTH (8L)
#define NUM_WATERMARK_METRICS (4)
#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#define MAX_WP_SLOTS (5)
#define CACHE_LINE_SZ (64)
#define ALT_STACK_SZ ((1L<<20) > 4 * SIGSTKSZ? (1L<<20): 4* SIGSTKSZ)
#define SAMPLES_POST_FULL_RESET_VAL (1)
#define IS_ALIGNED(address, alignment) (! ((size_t)(address) & (alignment-1)))
#define ADDRESSES_OVERLAP(addr1, len1, addr2, len2) (((addr1)+(len1) > (addr2)) && ((addr2)+(len2) > (addr1) ))
#define GET_OVERLAP_BYTES(a, a_len, b, b_len) ((a) >= (b)? MIN(a_len, (int64_t)(b_len) - ((int64_t)(a) - (int64_t)(b))) : MIN(b_len, (int64_t)(a_len) - ((int64_t)(b) - (int64_t)(a))))
#define FIRST_OVERLAPPED_BYTE_OFFSET_IN_FIRST(a, a_len, b, b_len) ((int64_t)(a) >= (int64_t)(b)? (0) : ((int64_t)b - (int64_t)a))
#define FIRST_OVERLAPPED_BYTE_OFFSET_IN_SECOND(a, a_len, b, b_len) ((int64_t)(b) >= (int64_t)(a)? (0) : ((int64_t)a - (int64_t)b))
#define IS_4_BYTE_ALIGNED(addr) (!((size_t)(addr) & (3)))
#define IS_8_BYTE_ALIGNED(addr) (!((size_t)(addr) & (7)))

#define APPROX_RATE (0.01)
#define MAX_FRAME_NUM (1024)
#define MAX_IP_DIFF (100000000)


#define EMSG(...) fprintf(stderr, __VA_ARGS__)
#define TD_GET()   ((WP_ThreadData_t *) pthread_getspecific(WP_ThreadData_key))


#if defined(PERF_EVENT_IOC_UPDATE_BREAKPOINT)
#define FAST_BP_IOC_FLAG (PERF_EVENT_IOC_UPDATE_BREAKPOINT)
#elif defined(PERF_EVENT_IOC_MODIFY_ATTRIBUTES)
#define FAST_BP_IOC_FLAG (PERF_EVENT_IOC_MODIFY_ATTRIBUTES)
#else
#endif

#define CHECK(x) ({int err = (x); \
if (err) { \
EMSG("%s: Failed with %d on line %d of file %s\n", strerror(errno), err, __LINE__, __FILE__); \
exit(0); }\
err;})

#if 0
/* ================================================================== */
// Debug register data types and structures
/* ================================================================== */
//typedef enum MergePolicy {AUTO_MERGE, NO_MERGE, CLIENT_ACTION} MergePolicy;
//typedef enum OverwritePolicy {OVERWRITE, NO_OVERWRITE} OverwritePolicy;
typedef enum VictimType {EMPTY_SLOT, NON_EMPTY_SLOT, NONE_AVAILABLE} VictimType;
#endif
// Data structure that is used when set a WP

typedef enum WP_Victim_t {WP_VICTIM_EMPTY_SLOT, WP_VICTIM_NON_EMPTY_SLOT, WP_VICTIM_NONE_AVAILABLE} WP_Victim_t;

typedef struct WP_RegisterInfo_t{
    uint32_t rtnAddr;
    void * va; // access virtual address
    int wpLength;
    int64_t startTime;
    int fileHandle;
    int threadId;
    bool isActive;
    void * mmapBuffer;
    void *userData;
    Event *sampledEvent;	//associated pebs event. useful to contruct context tree on watchpoint event.
    int sampleAccessLen;
    uint8_t sampleValue[MAX_WP_LENGTH]; // value
    uint64_t samplePostFull; // per watchpoint survival probability
} WP_RegisterInfo_t;

typedef struct WP_ThreadData_t{
    int lbrDummyFD __attribute__((aligned(CACHE_LINE_SZ)));
    stack_t ss;
    void * fs_reg_val;
    void * gs_reg_val;
    uint64_t samplePostFull;
    long numWatchpointTriggers;
    long numWatchpointImpreciseIP;
    long numWatchpointImpreciseAddressArbitraryLength;
    long numWatchpointImpreciseAddress8ByteLength;
    long numSampleTriggeringWatchpoints;
    long numWatchpointDropped;
    long numInsaneIP;
    struct drand48_data randBuffer;
    WP_RegisterInfo_t watchPointArray[MAX_WP_SLOTS];
    WP_TrapCallback_t fptr;
    char dummy[CACHE_LINE_SZ];
} WP_ThreadData_t;



#if 0
// Data structure that is captured when a WP triggers
typedef struct WatchPointTrigger{
    void * va;
    void * ctxt;
    void * pc;
    AccessType accessType;
    int accessLength; // access length
} WatchPointTrigger_t;
#endif

// Data structure that is maintained per WP armed
typedef struct WP_Config_t {
    bool isLBREnabled;
    bool isWPModifyEnabled;
    int signalDelivered;
    size_t pgsz;
    WP_ReplacementPolicy_t replacementPolicy;
    int maxWP;
    WP_PerfCallback_t userPerfPause, userPerfResume;
} WP_Config_t;


//************************************Static Methods****************************/

#if 0
static inline pid_t gettid() {
    return syscall(__NR_gettid);
}
#endif

static inline  uint64_t rdtsc(){
    unsigned int lo,hi;
    __asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi));
    return ((uint64_t)hi << 32) | lo;
}


#if 0
// calling perf event open system call
static inline long
perf_event_open(struct perf_event_attr *hw_event, pid_t pid, int cpu, int group_fd, unsigned long flags){
   return syscall(__NR_perf_event_open, hw_event, pid, cpu, group_fd, flags);
}
#endif


#if 0
extern WPConfig_t wpConfig;

typedef WPTriggerActionType (*WatchPointUpCall_t)(WatchPointInfo_t *, int, int, WatchPointTrigger_t*, int);

extern void InitConfig();
extern void WatchpointThreadInit(int threadId, WatchPointUpCall_t func);
extern void WatchpointThreadTerminate(int threadId);
extern bool SubscribeWatchpoint(int wpLength, WatchPointType type, void * va, int threadId);

/* ====================================================== */
// Client related data structures and functions
/* ====================================================== */
// Data structure that is given by clients to set a WP
typedef struct ThreadData{
    int lbrDummyFD __attribute__((aligned(CACHE_LINE_SZ)));
    stack_t ss;
    void * fs_reg_val;
    void * gs_reg_val;
    uint64_t samplePostFull;
    long numWatchpointTriggers;
    long numWatchpointImpreciseIP;
    long numWatchpointImpreciseAddressArbitraryLength;
    long numWatchpointImpreciseAddress8ByteLength;
    long numSampleTriggeringWatchpoints;
    long numWatchpointDropped;
    long numInsaneIP;
    struct drand48_data randBuffer;
    WatchPointInfo_t watchPointArray[MAX_WP_SLOTS];
    WatchPointUpCall_t fptr;
    char dummy[CACHE_LINE_SZ];
    int threadId;
} ThreadData_t;
/**********    system calls     *************/

#endif

#endif
