#include <atomic>
#include <sys/syscall.h>    /* For SYS_xxx definitions */


#include "platform.hpp"
#include "mytool.hpp"
#include "myagent.hpp"


// process and thread initialization callbacks
void *monitor_init_process(int *argc, char **argv, void *data)
{

        std::cout << "Within monitor init process pid: " << std::dec << getpid() << " pthread_self: "<< pthread_self() << " tid: " <<  syscall(SYS_gettid) << std::endl;
        MyAgent *agent = MyAgent::Instance();
        MyTool *tool = MyTool::Instance();
        tool->tool_processInit();

}

void monitor_fini_process(int how, void *data)
{

        std::cout << "Within monitor finish process" << std::endl;
        MyTool *tool = MyTool::Instance();
        tool->tool_processExit();
}


void *monitor_init_thread(int tid, void* data)
{

        std::cout << "Within monitor init thread. pid: " << std::dec << getpid() << " pthread_self: "<< pthread_self() << " tid: " <<  syscall(SYS_gettid) << std::endl;
        MyTool *tool = MyTool::Instance();
        tool->tool_threadInit();

}

void monitor_fini_thread(void* init_thread_data)
{

        std::cout << "Within monitor finish thread" << std::endl;
        MyTool *tool = MyTool::Instance();
        tool->tool_threadExit();

}

