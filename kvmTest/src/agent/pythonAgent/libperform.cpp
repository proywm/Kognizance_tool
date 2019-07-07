/**
* Wrapper
*/
#include <Python.h>
#include "pythonAgent/pyagent.hpp"

/*****************************************************************/
static PyObject * libperform_is_enabled(PyObject *module, PyObject *noargs) {
    if (pyAgent_is_enabled()) {
        Py_RETURN_TRUE;
    }
    Py_RETURN_FALSE;
}

static PyObject * libperform_print_stack(PyObject *module, PyObject *noargs) {
    if (pyAgent_print_stack()) {
        Py_RETURN_TRUE;
    }
    Py_RETURN_FALSE;
}

/**************************************************************/

static PyMethodDef PyAgentMethods[] = {
    {"is_enabled", libperform_is_enabled, METH_NOARGS,
        "Indicates if PyAgent is currently sampling."},
    {"print_stack", libperform_print_stack, METH_NOARGS,
        "Prints current callchain."},
    {NULL, NULL, 0, NULL}        /* Sentinel */
};


#if PY_MAJOR_VERSION >= 3
static struct PyModuleDef PyAgentModule = {
    PyModuleDef_HEAD_INIT,
    "_pyAgent",
    "",  // doc
    -1,  // size
    PyAgentMethods
};

PyMODINIT_FUNC PyInit_libperform(void)
{
    return PyModule_Create(&PyAgentModule);
}
#else
extern "C" {

PyMODINIT_FUNC initlibperform(void)
{
    m = Py_InitModule("libperform", PyAgentMethods);
    if (m == NULL)
        return;
}

}
#endif
