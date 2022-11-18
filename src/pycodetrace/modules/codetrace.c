#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include "codetrace.h"
#include "utils.h"

// Function declarations

static PyObject* codetrace_start();

static PyMethodDef CodeTrace_methods[] = {
    {"start", (PyCFunction)codetrace_start, METH_NOARGS, "start code trace"},
    {NULL, NULL, 0, NULL}
};

// ============================================================================
// Python interface
// ============================================================================

static struct PyModuleDef codetracemodule = {
    PyModuleDef_HEAD_INIT,
    .m_name = "pycodetrace.codetrace",
    .m_doc = "Python interface for the pycodetrace C library function",
    .m_size = -1
};

static PyObject*
codetrace_start()
{
    printf("Start, OK!\n");
    Py_RETURN_NONE;
}

// ============================================================================
// CodeTracer stuff
// ============================================================================
static PyObject *
CodeTracer_New(PyTypeObject *type, PyObject *args, PyObject *kwargs)
{
    CodeTracerObject *self = (CodeTracerObject *)type->tp_alloc(type, 0);
    self->max_stack_depth = 0;
    struct ObjectNode* node = (struct ObjectNode*) PyMem_Calloc(1, sizeof(struct ObjectNode));
    node->next = node;
    if (!node) {
        perror("Out of memory!");
        exit(-1);
    }
    self->node = node;
    self->output_file = NULL;
    self->total_track = 0;
    return (PyObject *)self;
}

static PyTypeObject CodeTraceType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "pycodetrace.CodeTracer",
    .tp_doc = "CodeTracer",
    .tp_basicsize = sizeof(CodeTracerObject),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_new = CodeTracer_New,
    .tp_methods = CodeTrace_methods
};

PyMODINIT_FUNC
PyInit_codetrace(void)
{
    // CodeTrace Module
    PyObject* m = NULL;

    if (PyType_Ready(&CodeTraceType) < 0) {
        return NULL;
    }

    m = PyModule_Create(&codetracemodule);
    if (m == NULL)
        return NULL;

    Py_INCREF(&CodeTraceType);
    if (PyModule_AddObject(m, "CodeTracer", (PyObject *)&CodeTraceType) < 0)
    {
        Py_DECREF(&CodeTraceType);
        Py_DECREF(&m);
        return NULL;
    }

    return m;
}
