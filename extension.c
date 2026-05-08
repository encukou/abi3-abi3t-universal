/*
Example module from PEP 793, further modified for compatibility
*/

#define Py_TARGET_ABI3T 0x030f0000
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

static PySlot extension_slots[] = {
    PySlot_STATIC_DATA(Py_mod_abi, &abi_info),
    PySlot_STATIC_DATA(Py_mod_name, "abi3_abi3t_universal"),
    PySlot_STATIC_DATA(Py_mod_doc, (void*)extension_doc),
    PySlot_STATIC_DATA(Py_mod_methods, extension_methods),
    PySlot_SIZE(Py_mod_state_size, sizeof(extension_state)),
    PySlot_STATIC_DATA(Py_mod_slots, ((PyModuleDef_Slot[]) {
        {Py_mod_exec, extension_exec},
        {Py_mod_gil, Py_MOD_GIL_NOT_USED},
        {0},
    })),
    PySlot_END
};

// Avoid "implicit declaration of function" warning:
PyMODEXPORT_FUNC PyModExport_abi3_abi3t_universal(PyObject *);

PyMODEXPORT_FUNC
PyModExport_abi3_abi3t_universal(PyObject *spec)
{
    return extension_slots;
}
