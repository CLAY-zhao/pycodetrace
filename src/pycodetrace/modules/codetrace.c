#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <frameobject.h>

#include "codetrace.h"
#include "utils.h"

// Function declarations

int codetrace_tracefuncdisabled(PyObject* obj, PyFrameObject* frame, int what, PyObject* arg);
int codetrace_tracefunc(PyObject* obj, PyFrameObject* frame, int what, PyObject* arg);
static PyObject* codetrace_watch(CodeTracerObject *self, PyObject *args);
static PyObject* codetrace_stop(CodeTracerObject* self, PyObject* args);

static PyMethodDef CodeTrace_methods[] = {
    {"watch", (PyCFunction)codetrace_watch, METH_VARARGS, "watch code"},
    {"stop", (PyCFunction)codetrace_stop, METH_VARARGS, "stop codetrace"},
    {NULL, NULL, 0, NULL}
};

PyFrameObject* curr_frame = NULL;
PyObject* inspect_module = NULL;

// ============================================================================
// Python interface
// ============================================================================

static struct PyModuleDef codetracemodule = {
    PyModuleDef_HEAD_INIT,
    .m_name = "pycodetrace.codetrace",
    .m_doc = "Python interface for the pycodetrace C library function",
    .m_size = -1
};

int
codetrace_tracefuncdisabled(PyObject* obj, PyFrameObject* frame, int what, PyObject* arg)
{
    CodeTracerObject* self = (CodeTracerObject*) obj;
    if (self->tracing) {
        PyEval_SetTrace(codetrace_tracefunc, obj);
        return codetrace_tracefunc(obj, frame, what, arg);
    }
    return 0;
}

int
codetrace_tracefunc(PyObject* obj, PyFrameObject* frame, int what, PyObject* arg)
{
    CodeTracerObject* self = (CodeTracerObject*) obj;
    if (!self->tracing) {
        PyEval_SetTrace(codetrace_tracefuncdisabled, obj);
        return 0;
    }

    curr_frame = frame->f_back; 

    int is_call = (what == PyTrace_CALL || what == PyTrace_C_CALL);
    int is_return = (what == PyTrace_RETURN || what == PyTrace_C_RETURN || what == PyTrace_C_EXCEPTION);
    int is_python = (what == PyTrace_CALL || what == PyTrace_RETURN);
    int is_c = (what == PyTrace_C_CALL || what == PyTrace_C_RETURN || what == PyTrace_C_EXCEPTION);

    PyObject* getframeinfo_method = PyObject_GetAttrString(inspect_module, "getframeinfo");
    if (!getframeinfo_method) {
        perror("Failed to access inspect.getframeinfo()");
        exit(-1);
    }

    PyObject* tuple = PyTuple_New(2);
    PyTuple_SetItem(tuple, 0, (PyObject*) curr_frame);
    PyTuple_SetItem(tuple, 1, PyLong_FromLong(1));
    PyObject* currentframeinfo = PyObject_CallObject(getframeinfo_method, tuple);
    if (!currentframeinfo) {
        perror("Failed to call inspect.getframeinfo()");
        exit(-1);
    }

    while (curr_frame) {
        curr_frame->f_trace = (PyObject*) codetrace_tracefunc;
        curr_frame = curr_frame->f_back;
    }
    
    PyObject* filename = PyObject_GetAttrString(currentframeinfo, "filename");
    PyObject* lineno = PyObject_GetAttrString(currentframeinfo, "lineno");
    PyObject* function = PyObject_GetAttrString(currentframeinfo, "function");
    PyObject* code_context = PyObject_GetAttrString(currentframeinfo, "code_context");

    Print_Obj(filename);
    Print_Obj(lineno);
    Print_Obj(function);
    Print_Obj(code_context);

    return 0;
}

static PyObject*
codetrace_watch(CodeTracerObject* self, PyObject* args)
{
    PyObject* obj = NULL;
    if (!PyArg_ParseTuple(args, "O", &obj)) {
        return NULL;
    }

    if (self->node->obj == NULL) {
        self->node->next = NULL;
        self->node->obj = obj;
    } else {
        struct ObjectNode* tmp = self->node;
        self->node = (struct ObjectNode*) PyMem_Calloc(1, sizeof(struct ObjectNode));
        self->node->next = tmp;
    }

    self->total_track++; // watch ele number

    if (!self->tracing) {
        self->tracing = 1;
        PyEval_SetTrace(codetrace_tracefunc, (PyObject*) self);
    }

    Py_RETURN_NONE;
}

static PyObject*
codetrace_stop(CodeTracerObject* self, PyObject* args)
{
    self->tracing = 0;
    curr_frame = NULL;
    PyEval_SetTrace(NULL, NULL);
    Py_RETURN_NONE;
}

// ============================================================================
// CodeTracer stuff
// ============================================================================
static PyObject *
CodeTracer_New(PyTypeObject* type, PyObject* args, PyObject* kwargs)
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
    self->tracing = 0;
    PyEval_SetTrace(codetrace_tracefuncdisabled, (PyObject*) self);
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

    inspect_module = PyImport_ImportModule("inspect");

    return m;
}
