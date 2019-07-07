/**
* PyAgent stack unwinder
*/

#include <ucontext.h>

#include "pythonAgent/pyagent.hpp"

#define SINGLE_BUF_SIZE (256 - 2 * sizeof(unsigned int))
#define MAX_NUM_BUFFERS 20
#define MAX_STACK_DEPTH   \
    ((SINGLE_BUF_SIZE - sizeof(int)) / sizeof(void *))

/*
* getting the PyThreadState from PyInterpreterState_ThreadHead for current thread
* PyThreadState: This data structure represents the state of a single thread. 
*/
PY_THREAD_STATE_T * _get_pystate_for_this_thread(void);


int pyagent_walk_and_record_python_stack_only(PY_STACK_FRAME_T *frame, Event *event,
                                          int max_depth, int depth, intptr_t pc);
int pyagent_walk_and_record_stack(PY_STACK_FRAME_T *frame, Event *event,
                              int max_depth, int signal, intptr_t pc);
int get_stack_trace(PY_THREAD_STATE_T * current, Event *event, int max_depth, intptr_t pc);

int pyagent_sample_stack(char **p, PY_THREAD_STATE_T * tstate, ucontext_t * uc);
