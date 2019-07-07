#include <iostream>
#include "pythonAgent/pyagent_stack_unwind.hpp"
#include "pyagent.hpp"

static volatile int spinlock;

void pyagent_aquire_lock(void) {
    while (__sync_lock_test_and_set(&spinlock, 1)) {
    }
}

void pyagent_release_lock(void) {
    __sync_lock_release(&spinlock);
}

void sigprof_handler_template()
{
	PY_THREAD_STATE_T * tstate = NULL;
	pyagent_aquire_lock();
	tstate = _get_pystate_for_this_thread();
	pyagent_release_lock();
	
	char* data[SINGLE_BUF_SIZE];
	int commit = pyagent_sample_stack(data, tstate, (ucontext_t*)NULL);
}

//void sig_handler_getContext(std::vector<ContextFrame*> *agentContext)
void sig_handler_getContext(Event *event)
{
	PY_THREAD_STATE_T * tstate = NULL;
        pyagent_aquire_lock();
//        tstate = _get_pystate_for_this_thread();
	tstate = PyGILState_GetThisThreadState();
        pyagent_release_lock();

//	std::vector<ContextFrame*> agentContext;
	int depth = get_stack_trace(tstate, event /*agentContext*/, MAX_STACK_DEPTH-1, (intptr_t)NULL);

}
