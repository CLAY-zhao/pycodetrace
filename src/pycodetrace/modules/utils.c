#include <Python.h>


void Print_Py(PyObject *o)
{
    PyObject* repr = PyObject_Repr(o);
    printf("%s\n", PyUnicode_AsUTF8(repr));
    Py_DECREF(repr);
}

void Print_Obj(PyObject *o)
{
    // print of cpython PyObject;
    const char *filename = "output.txt";
    FILE *fp;
    char buf[2048];
    errno_t err=0;

    // write data in file:
    errno = fopen_s(&fp, filename, "w");
    PyObject_Print(o, fp, Py_PRINT_RAW);
    fclose(fp);

    // read data from file:
    err = fopen_s(&fp, filename, "r");

    fgets(buf, 2048, fp);
    printf("buf: %s\n", buf);
    fclose(fp);
}
