
#include <Python.h>
#include <pythread.h>
#include <frameobject.h>
#include <unistd.h>
#include <vector>
#include "context.hpp"

#define PY_THREAD_STATE_T PyThreadState
#define PY_STACK_FRAME_T PyFrameObject
#define FRAME_STEP(f) f->f_back
#define FRAME_CODE(f) f->f_code

#  if PY_MAJOR_VERSION >= 3
      #define PyStr_AS_STRING PyBytes_AS_STRING
      #define PyStr_GET_SIZE PyBytes_GET_SIZE
      #define PyStr_NEW      PyUnicode_FromString
      #define PyStr_n_NEW      PyUnicode_FromStringAndSize
      #define PyLong_NEW     PyLong_FromSsize_t
      #define PyStr_Check    PyUnicode_Check
#  else
      #define PyStr_AS_STRING PyString_AS_STRING
      #define PyStr_GET_SIZE PyString_GET_SIZE
      #define PyStr_NEW      PyString_FromString
      #define PyStr_n_NEW      PyString_FromStringAndSize
      #define PyLong_NEW     PyInt_FromSsize_t
      #define PyLong_AsLong  PyInt_AsLong
      #define PyStr_Check    PyString_Check
#  endif



int pyAgent_is_enabled(void);
int pyAgent_print_stack(void);
void sigprof_handler_template();
//void sig_handler_getContext(std::vector<ContextFrame*> *agentContext);
void sig_handler_getContext(Event *event);
