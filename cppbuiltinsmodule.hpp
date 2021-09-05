#define PY_SSIZE_T_CLEAN
#include <Python.h>

static int
functools_copy_attr(PyObject *wrapper, PyObject *wrapped, PyObject *name)
{
    PyObject *value = PyObject_GetAttr(wrapped, name);
    if (value == NULL) {
        if (PyErr_ExceptionMatches(PyExc_AttributeError)) {
            PyErr_Clear();
            return 0;
        }
        return -1;
    }

    int res = PyObject_SetAttr(wrapper, name, value);
    Py_DECREF(value);
    return res;
}

// Similar to functools.wraps(wrapper, wrapped)
static int
functools_wraps(PyObject *wrapper, PyObject *wrapped)
{
#define COPY_ATTR(ATTR) \
    do { \
        _Py_IDENTIFIER(ATTR); \
        PyObject *attr = _PyUnicode_FromId(&PyId_ ## ATTR); \
        if (attr == NULL) { \
            return -1; \
        } \
        if (functools_copy_attr(wrapper, wrapped, attr) < 0) { \
            return -1; \
        } \
    } while (0) \

    COPY_ATTR(__module__);
    COPY_ATTR(__name__);
    COPY_ATTR(__qualname__);
    COPY_ATTR(__doc__);
    COPY_ATTR(__annotations__);
    return 0;

#undef COPY_ATTR
}