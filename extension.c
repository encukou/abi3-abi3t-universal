/*
Example module from PEP 793, further modified for compatibility
*/

#include <Python.h>

#define FTCOMPAT_MODNAME abi3_abi3t_universal
#include "ft_compat.h"

typedef struct {
    int value;
} extension_state;

static PyObject *
increment_value(PyObject *module, PyObject *_ignored)
{
    extension_state *state = PyModule_GetState(module);
    int result = ++(state->value);
    return PyLong_FromLong(result);
}

static PyMethodDef extension_methods[] = {
    {"increment_value", increment_value, METH_NOARGS},
    {NULL}
};

static int
extension_exec(PyObject *module) {
    extension_state *state = PyModule_GetState(module);
    state->value = -1;
    return 0;
}

PyDoc_STRVAR(extension_doc, "Example extension.");
PyABIInfo_VAR(abi_info);

static PyModuleDef_Slot extension_slots[] = {
    {Py_mod_abi, (void*)&abi_info},
    {Py_mod_name, "abi3_abi3t_universal"},
    {Py_mod_doc, (char*)extension_doc},
    {Py_mod_methods, extension_methods},
    {Py_mod_state_size, (void*)sizeof(extension_state)},
    {Py_mod_exec, (void*)extension_exec},
    {Py_mod_gil, (void*)Py_MOD_GIL_NOT_USED},
    {0}
};

// Avoid "implicit declaration of function" warning:
PyMODEXPORT_FUNC PyModExport_abi3_abi3t_universal(PyObject *);

PyMODEXPORT_FUNC
PyModExport_abi3_abi3t_universal(PyObject *spec)
{
    return extension_slots;
}
