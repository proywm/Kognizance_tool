#ifndef _DEVICE_H_
#define _DEVICE_H_

#include <asm/unistd.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <iostream>
#include <linux/hw_breakpoint.h>
#include <linux/perf_event.h>
#include <linux/kernel.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <ucontext.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <strings.h>
#include <time.h>
#include <math.h>
#include <semaphore.h>

#include <linux/hw_breakpoint.h>
#include <linux/perf_event.h>
#include <linux/kernel.h>

#include "context.hpp"

#define EBSDEVICE	1
#define PEBSDEVICE	2
#define BPDEVICE	3

#define MAX_POSSIBLE_DEVICES	128

#define CHECK(x) ({int err = (x); \
if (err) { \
fprintf(stderr, "%s: Failed with %d on line %d of file %s\n", strerror(errno), err, __LINE__, __FILE__); \
exit(-1); }\
err;})

static inline long perf_event_open(struct perf_event_attr *hw_event, pid_t pid, int cpu, int group_fd, unsigned long flags) {
    return syscall(__NR_perf_event_open, hw_event, pid, cpu, group_fd, flags);
}

/*
*
* class Device
* An abstract class
*
*/
class Device
{
private:
	int deviceFileId;
	int sessionId;
	int vcpu_fd;
	char * mmapBuffer;
	long long read_head;

	int pgsz;

	char * mmap_buffer();
	void unmap_buffer();

	int deviceType;
	int devSignum;

public:
	Device()
	{
		pgsz = getpagesize();
		read_head = 0;
	}
	~Device(){};
	int getDeviceFildId()	{ return deviceFileId;}
	int getDeviceSessionId() { return sessionId;}
	void setDeviceType(int dType) { deviceType = dType;}
	int getDeviceType() { return deviceType;}
	void *getDeviceMMapBuffer() { return mmapBuffer;}

	int registerDevice(int sessionId);
        void unRegisterDevice();

	void disable_device();
        void enable_device();

        void start_device()
        {
                isDisabled = 0;
                ioctl(deviceFileId, PERF_EVENT_IOC_REFRESH, 1);
        }
        void end_device()
        {
//		printf("Ending device %d . isDisabled? %d \n", deviceFileId, isDisabled);
                isDisabled = 1;
		disable_device();
//                ioctl(deviceFileId, PERF_EVENT_IOC_DISABLE, 0);
        }

	void dev_signal_handler(int signum, siginfo_t *info, void *context);
	void postAgentProcessing(Event *event, int signum, siginfo_t *info, void *context, int sessionId);
	Event * handle_perf_event();

	//Pure virtual function to be imeplemented by tool
	virtual int registerThisDevice(struct perf_event_attr *attr)=0;
	virtual void unRegisterThisDevice()=0;
	virtual void deviceOnEvent()=0;

	struct perf_event_attr pe_attr;
	int isDisabled = 0;

	//friend classes
	friend class BPDevice;
	friend class PEBSDevice;
	friend class EBSDevice;
};


#endif
