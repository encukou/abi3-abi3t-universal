
/* "PyInit_" shim for Python extension modules that use the PyModExport_* hook
 * (as proposed in PEP 793).
 *
 * In *one* C file (the one that defines the PyModExport_* hook):
 * - define FTCOMPAT_MODNAME to the name of your module
 * - include "ft_compat.h" (after <Python.h>)
 *
 * There are some limitations on the slots array:
 * - Sub-slots (Py_slot_subslots, Py_mod_slots) must not be used, except for
 *   a single optional `Py_mod_slots` used to set PyModuleDef.slots.
 *   Any 3.14 slots (create, exec, gil, multiple_interpreters) need to be here.
 * - Slots added in 3.15 (name, doc, methods, state_*, token, abi) must be
 *   specified directly in the top-level PySlot array.
 * - Additional slots (i.e. 3.16 & up) must be marked PySlot_OPTIONAL.
 * - Py_mod_token, if used, must be set to `&ftcompat_token` (which is defined
 *   in this header).
 * - Do not use PySlot_INTPTR.
 * - Everything must be static. Hovewer, the PySlot_STATIC flag is not checked.
 *
 * Note that the shim will call your PyModExport_* hook with NULL as the "spec"
 * argument.
 *
 * The shim should be ABI-compatible with Python 3.5 to 3.14 and free-threaded
 * builds of 3.13 & 3.14, *if* compiled with opaque PyObject structs (to
 * ensure that, for example, `Py_INCREF` calls an ABI function rather than
 * access a PyObject member).
 */

#include <string.h>     // for memset
#include <stdalign.h>   // for alignas

#ifndef PyMODINIT_FUNC
#error "This header must be included after Python.h"
#endif

#ifndef Py_LIMITED_API
#error "This header requires Py_LIMITED_API"
#endif

#if PY_VERSION_HEX < 0x030f0b01
#error "CPython 3.15.0b1 or above is required to build"
#endif

#ifndef FTCOMPAT_MODNAME
#error "Define FTCOMPAT_MODNAME to the name of the extension before including this header"
#endif

/* Always use old slot numbers */
#undef Py_mod_create
#undef Py_mod_exec
#undef Py_mod_multiple_interpreters
#undef Py_mod_gil
#define Py_mod_create 1
#define Py_mod_exec 2
#define Py_mod_multiple_interpreters 3
#define Py_mod_gil 4

#ifndef _Py_ALIGNED_DEF
#  if !defined(__alignas_is_defined) && defined(_MSC_VER)
#     define _Py_ALIGNED_DEF(N, T) __declspec(align(N)) T
#  else
#     define _Py_ALIGNED_DEF(N, T) alignas(N) alignas(T) T
#  endif
#endif

/* Shim structs, mirroring the stable ABI ("gil") and the
 * 3.14 free-threaded ABI ("ft")
 */

struct ftcompat_gil_PyObject {
    _Py_ALIGNED_DEF(4, Py_ssize_t) ob_refcnt;
    PyTypeObject *ob_type;
};

struct ftcompat_ft_PyObject {
    _Py_ALIGNED_DEF(4, uintptr_t) ob_tid;
    uint16_t ob_flags;
    char /* PyMutex */ ob_mutex;
    uint8_t ob_gc_bits;
    uint32_t ob_ref_local;
    Py_ssize_t ob_ref_shared;
    PyTypeObject *ob_type;
};

struct ftcompat_gil_PyModuleDef_Base {
    struct ftcompat_gil_PyObject ob_base;
    PyObject* (*m_init)(void);
    Py_ssize_t m_index;
    PyObject* m_copy;
};

struct ftcompat_ft_PyModuleDef_Base {
    struct ftcompat_ft_PyObject ob_base;
    PyObject* (*m_init)(void);
    Py_ssize_t m_index;
    PyObject* m_copy;
};

struct ftcompat_gil_PyModuleDef {
    struct ftcompat_gil_PyModuleDef_Base m_base;
    const char* m_name;
    const char* m_doc;
    Py_ssize_t m_size;
    PyMethodDef *m_methods;
    PyModuleDef_Slot *m_slots;
    traverseproc m_traverse;
    inquiry m_clear;
    freefunc m_free;
};

struct ftcompat_ft_PyModuleDef {
    struct ftcompat_ft_PyModuleDef_Base m_base;
    const char* m_name;
    const char* m_doc;
    Py_ssize_t m_size;
    PyMethodDef *m_methods;
    PyModuleDef_Slot *m_slots;
    traverseproc m_traverse;
    inquiry m_clear;
    freefunc m_free;
};

/* Union to reserve space for both variants of the PyModuleDef shim */

union ftcompat_PyModuleDef {
    struct ftcompat_gil_PyModuleDef def_gil;
    struct ftcompat_ft_PyModuleDef def_ft;
};

/* Helper to construct name using the user's extension module name */

#define FTCOMPAT_APPEND_MODNAME3(P, M) P ## M
#define FTCOMPAT_APPEND_MODNAME2(P, M) FTCOMPAT_APPEND_MODNAME3(P, M)
#define FTCOMPAT_APPEND_MODNAME(PREFIX) \
    FTCOMPAT_APPEND_MODNAME2(PREFIX, FTCOMPAT_MODNAME)

/* Forward definitions */

PyMODEXPORT_FUNC FTCOMPAT_APPEND_MODNAME(PyModExport_)(PyObject *);
PyMODINIT_FUNC FTCOMPAT_APPEND_MODNAME(PyInit_)(void);

/* Static PyModuleDef (and token) */

static union ftcompat_PyModuleDef ftcompat_token;

PyMODINIT_FUNC
FTCOMPAT_APPEND_MODNAME(PyInit_)(void)
{
    static int is_set_up;
    if (is_set_up) {
        // Take care to only set up the static PyModuleDef once.
        // (PyModExport might theoretically return different data each time.)
        return PyModuleDef_Init((void*)&ftcompat_token);
    }

    // Check the version (sadly, Py_Version is quite new)

    PyObject *hexversion_obj = PySys_GetObject("hexversion");  // borrowed ref
    if (!hexversion_obj) {
        PyErr_SetString(PyExc_ImportError, "sys.hexversion not found");
        return NULL;
    }
    long hexversion = PyLong_AsLong(hexversion_obj);
    if (hexversion < 0 && PyErr_Occurred()) {
        return NULL;
    }
    if (hexversion > 0x030f0000) {
        PyErr_SetString(PyExc_ImportError, "PyInit_* used on Python 3.15+");
        return NULL;
    }

    // Determine if we have free-threaded ABI

    PyObject *obj_size_obj = PyObject_CallMethod(Py_None, "__sizeof__", "");
    if (!obj_size_obj) {
        return NULL;
    }
    Py_ssize_t obj_size = PyLong_AsSsize_t(obj_size_obj);
    Py_DECREF(obj_size_obj);

    int freethreading_abi;
    if (obj_size == (Py_ssize_t)sizeof(struct ftcompat_gil_PyObject)) {
        freethreading_abi = 0;
    }
    else if (obj_size == (Py_ssize_t)sizeof(struct ftcompat_ft_PyObject)) {
        freethreading_abi = 1;
    }
    else {
         if (!PyErr_Occurred()) {
            PyErr_SetString(PyExc_ImportError, "Unknown object size");
         }
         return NULL;
    }

    // One more safety check

    if (freethreading_abi && hexversion < 0x030d0000) {
        PyErr_SetString(PyExc_ImportError, "'t' ABI found below Python 3.13");
        return NULL;
    }

    // Call the new export hook and construct a moduledef shim from it

    PySlot *slot = FTCOMPAT_APPEND_MODNAME(PyModExport_)(NULL);

    for (/* slot set above */; slot->sl_id; slot++) {
        if (slot->sl_flags & PySlot_INTPTR) {
            PyErr_SetString(
                PyExc_SystemError,
                "PySlot_INTPTR is not allowed.");
            return NULL;
        }
        switch (slot->sl_id) {
        // Set PyModuleDef members from slots. These slots must come first.
#       define COPYSLOT_CASE(SLOT, DEF_MEMBER, SL_MEMBER, TYPE)             \
            case SLOT:                                                      \
                if (freethreading_abi) {                                    \
                    if (ftcompat_token.def_ft.DEF_MEMBER) {                 \
                        PyErr_SetString(PyExc_SystemError,                  \
                                        #SLOT " must be not be repeated");  \
                        goto error;                                         \
                    }                                                       \
                    ftcompat_token.def_ft.DEF_MEMBER =                      \
                        (TYPE)(slot->SL_MEMBER);                            \
                } else {                                                    \
                    if (ftcompat_token.def_gil.DEF_MEMBER) {                \
                        PyErr_SetString(PyExc_SystemError,                  \
                                        #SLOT " must be not be repeated");  \
                        goto error;                                         \
                    }                                                       \
                    ftcompat_token.def_gil.DEF_MEMBER =                     \
                        (TYPE)(slot->SL_MEMBER);                            \
                }                                                           \
                break;                                                      \
            /////////////////////////////////////////////////////////////////
        COPYSLOT_CASE(Py_mod_name, m_name, sl_ptr, char*)
        COPYSLOT_CASE(Py_mod_doc, m_doc, sl_ptr, char*)
        COPYSLOT_CASE(Py_mod_state_size, m_size, sl_size, Py_ssize_t)
        COPYSLOT_CASE(Py_mod_methods, m_methods, sl_func, PyMethodDef*)
        COPYSLOT_CASE(Py_mod_state_traverse, m_traverse, sl_func, traverseproc)
        COPYSLOT_CASE(Py_mod_state_clear, m_clear, sl_func, inquiry)
        COPYSLOT_CASE(Py_mod_state_free, m_free, sl_func, freefunc)
        COPYSLOT_CASE(Py_mod_slots, m_slots, sl_ptr, PyModuleDef_Slot*)
#       undef COPYSLOT_CASE
        case Py_mod_token:
            // With PyInit_, the PyModuleDef is used as the token.
            if (slot->sl_ptr != &ftcompat_token) {
                PyErr_SetString(PyExc_SystemError,
                                "Py_mod_token must be set to "
                                "&ftcompat_token");
                goto error;
            }
            break;
        case Py_mod_abi:
            // Py_mod_abi is ignored.
            break;
        case Py_slot_subslots:
            PyErr_SetString(
                PyExc_SystemError,
                "Py_slot_subslots is not allowed.");
            return NULL;
        default:
            if (slot->sl_flags & PySlot_OPTIONAL) {
                break;
            }
            PyErr_SetString(
                PyExc_SystemError,
                "Slot %d must be marked PySlot_OPTIONAL");
            return NULL;
        }
    }
    is_set_up = 1;
    return PyModuleDef_Init((void*)&ftcompat_token);

error:
    memset(&ftcompat_token, 0, sizeof(ftcompat_token));
    return NULL;
}
#undef FTCOMPAT_APPEND_MODNAME
#undef FTCOMPAT_APPEND_MODNAME2
#undef FTCOMPAT_APPEND_MODNAME3
