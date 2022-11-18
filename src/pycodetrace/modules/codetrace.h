#ifndef __CODETRACE_H__
#define __CODETRACE_H__

#include <Python.h>

struct ObjectNode
{
    struct ObjectNode* next;
    PyObject* obj;
};

typedef struct
{
    PyObject_HEAD
    int max_stack_depth;
    struct ObjectNode* node;
    int total_track;
    PyObject* output_file;
} CodeTracerObject;

#endif