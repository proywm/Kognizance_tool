/**
* PyAgent stack unwinder
*/
#include <iostream>
#include "pythonAgent/pyagent.hpp"
#include "pythonAgent/pyagent_stack_unwind.hpp"

#include "pyBytecodeIterator.hpp"

#include "pyopcode.hpp"

//extern int _Py_OpcodeStackEffect(int opcode, int oparg);

/*
* getting the PyThreadState from PyInterpreterState_ThreadHead for current thread
* PyThreadState: This data structure represents the state of a single thread. 
*/
PY_THREAD_STATE_T * _get_pystate_for_this_thread(void) {
    // see issue 116 on github.com/vmprof/vmprof-python.
    // PyGILState_GetThisThreadState(); can hang forever
    //
    PyInterpreterState * istate;
    PyThreadState * state;
    long mythread_id;

    mythread_id = PyThread_get_thread_ident();
    istate = PyInterpreterState_Head();
    if (istate == NULL) {
        fprintf(stderr, "WARNING: interp state head is null (for thread id %ld)\n", mythread_id);
        return NULL;
    }
    // fish fish fish, it will NOT lock the keymutex in pythread
    do {
        state = PyInterpreterState_ThreadHead(istate);
        do {
            if (state->thread_id == mythread_id) {
		std::cout << "Got this thread's pystate"<< std::endl;
                return state;
            }
        } while ((state = PyThreadState_Next(state)) != NULL);
    } while ((istate = PyInterpreterState_Next(istate)) != NULL);

    // uh? not found?
    fprintf(stderr, "WARNING: cannot find thread state (for thread id %ld), sample will be thrown away\n", mythread_id);
    return NULL;
}

int pyagent_sample_stack(char **p, PY_THREAD_STATE_T * tstate, ucontext_t * uc)
{
    int depth;
//    depth = get_stack_trace(tstate, (void**) p, MAX_STACK_DEPTH-1, (intptr_t)NULL);

    // useful for tests (see test_stop_sampling)
#ifndef RPYTHON_LL2CTYPES
    if (depth == 0) {
        return 0;
    }
#endif
    return 1;
}

int get_stack_trace(PY_THREAD_STATE_T * current, Event *event/*std::vector<ContextFrame*> *agentContext*/, int max_depth, intptr_t pc)
{
    PY_STACK_FRAME_T * frame;
    if (current == NULL) {
        fprintf(stderr, "WARNING: get_stack_trace, current is NULL\n");
        return 0;
    }

    frame = current->frame;

    if (frame == NULL) {
        fprintf(stderr, "WARNING: get_stack_trace, frame is NULL\n");
        return 0;
    }
    return pyagent_walk_and_record_stack(frame, event/*agentContext*/, max_depth, 1, pc);
}

int pyagent_walk_and_record_stack(PY_STACK_FRAME_T *frame, Event *event/*std::vector<ContextFrame*> *agentContext*/,
                              int max_depth, int signal, intptr_t pc) {

    // called in signal handler
    //
    // This function records the stack trace for a python program. It also
    // tracks native function calls if libunwind can be found on the system.
    //
    // The idea is the following (in the native case):
    //
    // 1) Remove frames until the signal frame is found (skipping it as well)
    // 2) if the current frame corresponds to PyEval_EvalFrameEx (or the equivalent
    //    for each python version), the jump to 4)
    // 3) jump to 2)
    // 4) walk each python frame and record it
    //
    //
    // There are several cases that need to be taken care of.
    //
    // CPython supports line profiling, PyPy does not. At the same time
    // PyPy saves the information of an address in the same way as line information
    // is saved in CPython. _write_python_stack_entry for details.
    //
	return pyagent_walk_and_record_python_stack_only(frame, event/*agentContext*/, max_depth, 0, pc);
}

int _per_loop()
{
	return 2;
}
int pyagent_walk_and_record_python_stack_only(PY_STACK_FRAME_T *frame, Event *event/*std::vector<ContextFrame*> *agentContext*/,
                                          int max_depth, int depth, intptr_t pc)
{
    while ((depth + _per_loop()) <= max_depth && frame) {
	int len;
    	int addr = 0;
	int bytecodeiter = 0;
    	int j = 0;
    	uint64_t line = 0;
	char *lnotab;
	
	lnotab = PyStr_AS_STRING(frame->f_code->co_lnotab);
	if (lnotab != NULL) {
#if 1
            line = (uint64_t)frame->f_lineno;

            len = (int)PyStr_GET_SIZE(frame->f_code->co_lnotab);

            for (j = 0; j < len; j += 2) {
                addr += lnotab[j];
		bytecodeiter++;
                if (addr > frame->f_lasti) {
                    break;
                }
                line += lnotab[j+1];
            }
#endif
	    
	    int lnum = PyCode_Addr2Line(frame->f_code, frame->f_lasti);
	    ContextFrame *contextFrame = new ContextFrame();
	    contextFrame->binary_addr = addr;
            contextFrame->method_name = PyUnicode_AsUTF8(frame->f_code->co_name);
            contextFrame->source_file = PyUnicode_AsUTF8(frame->f_code->co_filename);
	    contextFrame->src_lineno = line;
	    if(event->eventContext_agent.size() == 0 || !(*event->eventContext_agent.back() == *contextFrame))
	    {
            	event->eventContext_agent.push_back(contextFrame);
	    }
            depth = depth + 1;
        } else {
            depth = depth + 1;
        }
#if 1

	PyBytecodeIterator iter(frame->f_code->co_code);
	int prevOpCode = -1;
	for (; !iter.Done() && !iter.Error(); iter.Advance()) {
		if(addr == iter.CurIndex())
			break;
		prevOpCode = iter.Opcode();
	}
	event->opcode = iter.Opcode();
	if(event->fixSkid)
	{
		event->fixedIP = iter.CurIndex();
		event->isFixed = true;
		event->opcode = prevOpCode;
	}
	if((STORE_SUBSCR == prevOpCode) || (STORE_NAME == prevOpCode) || (STORE_ATTR == prevOpCode) || (STORE_GLOBAL == prevOpCode) || (STORE_FAST == prevOpCode) || (STORE_ANNOTATION == prevOpCode) || (STORE_DEREF == prevOpCode))
		event->accessType = STORE;
	else if((LOAD_BUILD_CLASS == prevOpCode) || (LOAD_CONST == prevOpCode) || (LOAD_NAME == prevOpCode) || (LOAD_ATTR == prevOpCode) || (LOAD_GLOBAL == prevOpCode) || (LOAD_FAST == prevOpCode) || (LOAD_CLOSURE == prevOpCode) || (LOAD_DEREF == prevOpCode) || (LOAD_CLASSDEREF == prevOpCode))
		event->accessType = LOAD;
#endif
	frame = FRAME_STEP(frame);
    }


    return depth;
}

