#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <python3.8/structmember.h>
#include <python3.8/frameobject.h>
#include <string>
#include <stdio.h>
#include <unordered_map>
#include <string>

static PyObject * c_abs_func(PyObject *self, PyObject *args){
    signed long long num;
    if (!PyArg_ParseTuple(args, "L", &num)){
        return NULL;
    }
    if (num < 0){
        num *= -1;
    }
    return PyLong_FromLongLong(num);
}

// Will use METH_O because we don't need to parse args into C types
static PyObject * c_all_func(PyObject *self, PyObject *seq){
    PyObject *iterator, *item;
    iterator = PyObject_GetIter(seq);
    Py_XDECREF(seq);
    if (iterator == NULL){
        PyObject *pystring;
        pystring = PyUnicode_FromFormat("'%s' object is not iterable", seq->ob_type->tp_name);
        PyErr_SetObject(PyExc_TypeError, pystring);
        return NULL;
    }
    int check;
    while ((item = PyIter_Next(iterator)) != NULL){
        check = PyObject_Not(item);
        Py_XDECREF(item);
        if (check == -1){
            PyObject *pystring = PyUnicode_FromString("Unable to get Truth values");
            PyErr_SetObject(PyExc_ValueError, pystring);
            return NULL;
        }
        else if (check){
            Py_RETURN_FALSE;
        }
    }
    Py_RETURN_TRUE;
}

//Use METH_O
static PyObject * c_any_func(PyObject *self, PyObject *seq){
    PyObject *iterator, *item;
    iterator = PyObject_GetIter(seq);
    Py_XDECREF(seq);
    if (iterator == NULL){
        PyObject *pystring;
        pystring = PyUnicode_FromFormat("'%s' object is not iterable", seq->ob_type->tp_name);
        PyErr_SetObject(PyExc_TypeError, pystring);
        return NULL;
    }
    int check;
    while ((item = PyIter_Next(iterator)) != NULL){
        check = PyObject_IsTrue(item);
        Py_XDECREF(item);
        if (check == -1){
            PyObject *pystring = PyUnicode_FromString("Unable to get Truth values");
            PyErr_SetObject(PyExc_ValueError, pystring);
            return NULL;
        }
        else if (check){
            Py_RETURN_TRUE;
        }
    }
    Py_RETURN_FALSE;
}

void my_base_converter(signed long long *num, char base_char, std::string *result){
    int base;
    std::string header;
    if (base_char == 'b'){
        header = "0b";
        base = 2;
    }
    else {
        header = "0o";
        base = 8;
    }
    if (*num < 0){
        header = '-' + header;
        *num *= -1;
    }
    else if (*num == 0){
        header += '0';
        *result = header;
        return;
    }
    while (*num != 0){
        *result = std::to_string(*num % base) + *result;
        *num /= base;
    }
    *result = header + *result;
    return;
}

void my_base_converter(signed long long *num, std::string *result){
    std::unordered_map<int, char> num_map = {{0, '0'}, {1, '1'}, {2, '2'}, {3, '3'}, {4, '4'},
                                             {5, '5'}, {6, '6'}, {7, '7'}, {8, '8'}, {9, '9'},
                                             {10, 'a'}, {11, 'b'}, {12, 'c'}, {13, 'd'},
                                             {14, 'e'}, {15, 'f'}};
    int base = 16;
    std::string header = "0x";
    if (*num < 0){
        header = '-' + header;
        *num *= -1;
    }
    else if (*num == 0){
        header += '0';
        *result = header;
        return;
    }
    while (*num != 0){
        *result = num_map[*num % base] + *result;
        *num /= base;
    }
    *result = header + *result;
    return;
}

static PyObject * c_bin_func(PyObject *self, PyObject *args){
    signed long long num;
    if (!PyArg_ParseTuple(args, "L", &num)){
        return NULL;
    }
    std::string result = "";
    my_base_converter(&num, 'b', &result);
    return PyUnicode_FromString(result.c_str());
}

//METH_O
static PyObject * c_api_bin_func(PyObject *self, PyObject *num){
    PyObject * result = PyNumber_ToBase(num, 2);
    if (result == NULL){
        return NULL;
    }
    return PyObject_Str(result);
}

//METH_O
static PyObject * c_bool_func(PyObject *self, PyObject *item){
    if (PyObject_HasAttrString(item, "__bool__")){
        PyObject *func = PyObject_GetAttrString(item, "__bool__");
        return PyObject_CallObject(func, NULL);
    }
    else if (PyObject_HasAttrString(item, "__len__")){
        Py_ssize_t length = PyObject_Length(item);
        if (length == -1){
            PyObject *pystring = PyUnicode_FromString("Unable to call __len__");
            PyErr_SetObject(PyExc_Exception, pystring);
            return NULL;
        }
        if (length){
            Py_RETURN_TRUE;
        }
        Py_RETURN_FALSE;
    }
    else{
        Py_RETURN_TRUE;
    }
}


//METH_O
static PyObject * c_bool_api_func(PyObject *self, PyObject *item){
    if (PyObject_IsTrue(item)){
        Py_RETURN_TRUE;
    }
    Py_RETURN_FALSE;
}

//METH_O
static PyObject * c_callable_func(PyObject *self, PyObject *item){
    if (PyObject_HasAttrString(item, "__call__")){
        Py_RETURN_TRUE;
    }
    Py_RETURN_FALSE;
}

//METH_O
static PyObject * c_callable_api_func(PyObject *self, PyObject *item){
    if (PyCallable_Check(item)){
        Py_RETURN_TRUE;
    }
    Py_RETURN_FALSE;
}

static PyObject * c_dir_func(PyObject *self, PyObject *args){
    PyObject *item = NULL;
    PyObject *attrs, *result;
    if (!(PyArg_ParseTuple(args, "|O", &item))){
        PyObject *pystring = PyUnicode_FromString("Error Occurred!");
        PyErr_SetObject(PyExc_Exception, pystring);
        return NULL;
    }
    if (item == NULL){
        PyFrameObject *cur_frame = PyEval_GetFrame();
        PyObject *f_locals;
        if (cur_frame->f_back != NULL){
            f_locals = cur_frame->f_back->f_locals;
        }
        else {
            f_locals = cur_frame->f_locals;
        }
        attrs = PyDict_Keys(f_locals);
    }
    else if (PyObject_HasAttrString(item, "__dir__")){
        PyObject *func = PyObject_GetAttrString(item, "__dir__");
        attrs = PyObject_CallObject(func, NULL);
    }
    else {
        attrs = PyList_New(0);
        if (PyObject_HasAttrString(item, "__dict__")){
            _PyList_Extend((PyListObject *)attrs, PyObject_GetAttrString(item, "__dict__"));
        }
        if (PyObject_HasAttrString(item, "__slots__")){
            PyObject *slot = PyObject_GetAttrString(item, "__slots__");
            if (PySequence_Check(slot)){
                if (PyUnicode_Check(slot)){
                    PyList_Append(attrs, slot);
                }
                else {
                    _PyList_Extend((PyListObject *)attrs, slot);
                }
            }
            else {
                PyList_Append(attrs, slot);
            }
        }
    }
    result = PyList_New(0);
    _PyList_Extend((PyListObject *)result, PySet_New(attrs));
    PyList_Sort(result);
    Py_XDECREF(attrs);
    return result;
}

static PyObject * c_dir_api_func(PyObject *self, PyObject *args){
    PyObject *item = NULL;
    if (!(PyArg_ParseTuple(args, "|O", &item))){
        PyObject *pystring = PyUnicode_FromString("Error Occurred!");
        PyErr_SetObject(PyExc_Exception, pystring);
        return NULL;
    }
    PyObject *result = PyObject_Dir(item);
    return result;
}

// Module Level Function Registry 
static PyMethodDef CPPBuiltinsMethods[] = {
    // Python function name, Actual function, function flag, docstring
    {"abs", c_abs_func, METH_VARARGS, "abs() in C++."},
    {"all", c_all_func, METH_O, "all() in C++."},
    {"any", c_any_func, METH_O, "any() in C++."},
    {"bin", c_bin_func, METH_VARARGS, "bin() in C++."},
    {"bin_api", c_api_bin_func, METH_O, "bin() using C API."},
    {"bool", c_bool_func, METH_O, "bool() in C++."},
    {"bool_api", c_bool_api_func, METH_O, "bool() using C API."},
    {"callable", c_callable_func, METH_O, "callable() in C++"},
    {"callable_api", c_callable_api_func, METH_O, "callable() using C API."},
    {"dir", c_dir_func, METH_VARARGS, "dir() in C++."},
    {"dir_api", c_dir_api_func, METH_VARARGS, "dir() using C API."},
    {NULL, NULL, 0, NULL}
};

/*Module Structure*/
static struct PyModuleDef cppbuiltinsmodule = {
    PyModuleDef_HEAD_INIT,
    "cppbuiltins",   /* name of module */
    "A rewrite of some Python builtins.", /* Doc string, may be NULL */
    -1, /* Size of per-interpreter state or -1 */
    CPPBuiltinsMethods /* Method table */
};

//Module initialization function
PyMODINIT_FUNC
PyInit_cppbuiltins(void) {
    /*if (PyType_Ready(&Py_ClassMethodType) < 0){
        std::printf("Failure\n");
        return NULL;
    }*/
    PyObject* m = PyModule_Create(&cppbuiltinsmodule);
    if (m == NULL) {
        return NULL;
    }
    /*Py_INCREF(&Py_ClassMethodType);
    if (PyModule_AddObject(m, "classmethod", (PyObject *) &Py_ClassMethodType) < 0){
        Py_DECREF(&Py_ClassMethodType);
        Py_DECREF(m);
        return NULL;
    }*/
    return m;
}

