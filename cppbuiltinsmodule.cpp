#include "cppbuiltinsmodule.hpp"
#ifndef PY_SSIZE_T_CLEAN
    #define PY_SSIZE_T_CLEAN
    #include <Python.h>
#endif
#include <python3.9/structmember.h>
#include <python3.9/frameobject.h>
//#include <python3.9/internal/pycore_pystate.h>
#include <string>
#include <iostream>
#include <unordered_map>
#include <string>
#include <tuple>
#include <vector>

#define PyKwargFunction (PyCFunction)(void(*)(void))
#define llong unsigned long long

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
    PyObject *result, *temp = PyNumber_ToBase(num, 2);
    if (temp == NULL){
        return NULL;
    }
    result = PyObject_Str(temp);
    Py_XDECREF(temp);
    return result;
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

/*functools_wraps for functool.wraps 
 Objects/funcobject.c*/

typedef struct {
    PyObject_HEAD
    PyObject *callable;
    PyObject *dict;
} MyClassMethod;

//PyType_GenericNew, PyType_GenericAlloc

int MyClassMethod_Init(MyClassMethod *self, PyObject *args, PyObject *kwargs){
    PyObject *func;
    if (!PyArg_ParseTuple(args, "O", &func)){
        return -1;
    }
    Py_XINCREF(func);
    if ((functools_wraps((PyObject *)self, func)) < 0){
        Py_XDECREF(func);
        return -1;
    }
    self->callable = func;
    return 0;
}

static PyObject * MyClassMethod_Descr_Get(MyClassMethod *self, PyObject *obj, PyObject *owner){
    if (owner == NULL){
        owner = (PyObject *)(Py_TYPE(obj));
    }
    return PyMethod_New(self->callable, owner);
}

static void MyClassMethod_Dealloc(MyClassMethod *self){
    Py_XDECREF(self->callable);
    Py_XDECREF(self->dict);
    Py_TYPE(self)->tp_free((PyObject *)self);
}

static int MyClassMethod_Clear(MyClassMethod *self){
    Py_CLEAR(self->callable);
    Py_CLEAR(self->dict);
    return 0;
}

static int MyClassMethod_Vist(MyClassMethod *self, visitproc visit, void *arg){
    Py_VISIT(self->callable);
    Py_VISIT(self->dict);
    return 0;
}

static PyMemberDef MyClassMethod_Members[] = {
    {"__func__", T_OBJECT_EX, offsetof(MyClassMethod, callable), READONLY},
    {"__wrapped__", T_OBJECT_EX, offsetof(MyClassMethod, callable), READONLY},
    {NULL}
};

static PyGetSetDef MyClassMethod_GetSet[] = {
    {"__dict__", PyObject_GenericGetDict, PyObject_GenericSetDict, NULL, NULL},
    {NULL}
};

PyTypeObject MyClassMethod_Type = {
    PyVarObject_HEAD_INIT(&PyType_Type, 0)
    .tp_name = "cppbuiltins.classmethod",
    .tp_basicsize = sizeof(MyClassMethod),
    .tp_itemsize = 0,
    .tp_dealloc = (destructor)MyClassMethod_Dealloc,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE | Py_TPFLAGS_HAVE_GC,
    .tp_traverse = (traverseproc)MyClassMethod_Vist,
    .tp_clear = (inquiry)MyClassMethod_Clear,
    .tp_members = MyClassMethod_Members,
    .tp_getset = MyClassMethod_GetSet,
    .tp_descr_get = (descrgetfunc)MyClassMethod_Descr_Get,
    .tp_dictoffset = offsetof(MyClassMethod, dict),
    .tp_init = (initproc)MyClassMethod_Init,
    .tp_alloc = PyType_GenericAlloc,
    .tp_new = PyType_GenericNew,
    .tp_free = PyObject_GC_Del, 
};

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
        attrs = PyObject_CallOneArg(func, item);
        if (PyErr_Occurred()){
            Py_XDECREF(attrs);
            PyErr_Clear();
            attrs = PyObject_CallNoArgs(func);
        }
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

void propogate_div_err(const char *obj1, const char *obj2){
    PyErr_Clear();
    std::string name1 = obj1;
    std::string name2 = obj2;
    PyObject *pystring;
    std::string msg = "unsupported operand type(s) for divmod(): '%s' and '%s'";
    pystring = PyUnicode_FromFormat(msg.c_str(), name1.c_str(), name2.c_str());
    PyErr_SetObject(PyExc_TypeError, pystring);
}

static PyObject * c_divmod_func(PyObject *self, PyObject *args){
    PyObject *dividend, *divisor, *divfunc, *q, *r, *result;
    if (!PyArg_ParseTuple(args, "OO", &dividend, &divisor)){
        PyObject *pystring = PyUnicode_FromString("Missing Valid Argument");
        PyErr_SetObject(PyExc_TypeError, pystring);
        return NULL;
    }
    auto normal_div = std::make_tuple(dividend, divisor, "__floordiv__", "__mod__");
    auto reflective_div = std::make_tuple(divisor, dividend, "__rfloordiv__", "__rmod__");
    std::vector<decltype(normal_div)> args_vector;
    args_vector.reserve(2);
    args_vector.push_back(normal_div);
    args_vector.push_back(reflective_div);
    std::string divname, modname;
    for (int i = 0; i < 2; i++){
        std::tie(dividend, divisor, divname, modname) = args_vector[i];
        if (PyObject_HasAttrString(dividend, divname.c_str())){
            divfunc = PyObject_GetAttrString(dividend, divname.c_str());
            q = PyObject_CallOneArg(divfunc, divisor);
            Py_XDECREF(divfunc);
            if (PyErr_Occurred()){
                Py_XDECREF(q);
                propogate_div_err(dividend->ob_type->tp_name, divisor->ob_type->tp_name);
                return NULL;
            }
            if (PyObject_HasAttrString(dividend, modname.c_str())){
                divfunc = PyObject_GetAttrString(dividend, modname.c_str());
                r = PyObject_CallOneArg(divfunc, divisor);
                Py_XDECREF(divfunc);
                if (PyErr_Occurred()){
                    Py_XDECREF(q);
                    Py_XDECREF(r);
                    propogate_div_err(dividend->ob_type->tp_name, divisor->ob_type->tp_name);
                    return NULL;
                }
            }
            else {
                Py_XDECREF(q);
                propogate_div_err(dividend->ob_type->tp_name, divisor->ob_type->tp_name);
                return NULL;
            }
            if (PyObject_RichCompareBool(q, Py_NotImplemented, Py_EQ) ||
                PyObject_RichCompareBool(r, Py_NotImplemented, Py_EQ)){
                    if (i == 0){
                        continue;
                    }
                    else {
                        Py_XDECREF(q);
                        Py_XDECREF(r);
                        propogate_div_err(dividend->ob_type->tp_name, divisor->ob_type->tp_name);
                        return NULL;
                    }
                }
            break;
        }
        else {
            propogate_div_err(dividend->ob_type->tp_name, divisor->ob_type->tp_name);
            return NULL;
        }
    }
    result = PyTuple_Pack(2, q, r);
    return result;
}

static PyObject * c_divmod_api_func(PyObject *self, PyObject *args){
    PyObject *dividend, *divisor, *result;
    if (!PyArg_ParseTuple(args, "OO", &dividend, &divisor)){
        PyObject *pystring = PyUnicode_FromString("Missing Valid Argument");
        PyErr_SetObject(PyExc_TypeError, pystring);
        return NULL;
    }
    result = PyNumber_Divmod(dividend, divisor);
    return result;
}

typedef struct {
    PyObject_HEAD
    PyObject *iterable;
    Py_ssize_t counter;
} PyEnumerateObject;

static PyObject * PyEnumerate_New(PyTypeObject *type, PyObject *args, PyObject *kwargs){
    PyEnumerateObject *new_obj = (PyEnumerateObject *)type->tp_alloc(type, 0);
    if (!new_obj){
        return NULL;
    }
    return (PyObject *) new_obj;
}

int PyEnumerate_Init(PyEnumerateObject *self, PyObject *args, PyObject *kwargs){
    self->counter = 0;
    PyObject *seq;
    static char *kwlist[] = {(char *)"", (char *)"start", NULL};
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O|n", kwlist, &seq, &(self->counter))){
        return -1;
    }
    if (!(self->iterable = PyObject_GetIter(seq))){
        std::string msg = "'%s' object is not iterable";
        PyObject *pystring = PyUnicode_FromFormat(msg.c_str(), self->iterable->ob_type->tp_name);
        PyErr_SetObject(PyExc_TypeError, pystring);
        return -1;
    }
    return 0;
}

static PyObject * PyEnumerate_Iter(PyEnumerateObject *self){
    Py_XINCREF(self);
    return (PyObject *)self;
}

static PyObject * PyEnumerate_Next(PyEnumerateObject *self){
    self->counter += 1;
    PyObject *item;
    if (!(item = PyIter_Next(self->iterable))){
        if (PyErr_Occurred()){
            return NULL;
        }
        PyErr_SetString(PyExc_StopIteration, "");
        return NULL;
    }
    return PyTuple_Pack(2, PyLong_FromSsize_t(self->counter - 1), item);
}

static void PyEnumerate_Dealloc(PyEnumerateObject *self){
    Py_XDECREF(self->iterable);
    Py_TYPE(self)->tp_free((PyObject *) self);
}

static PyTypeObject PyEnumerate_Type {
    PyObject_HEAD_INIT(NULL)
    .tp_name = "cppbuiltins.enumerate",
    .tp_basicsize = sizeof(PyEnumerateObject),
    .tp_itemsize = 0,
    .tp_dealloc = (destructor) PyEnumerate_Dealloc,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_iter = (getiterfunc) PyEnumerate_Iter,
    .tp_iternext = (iternextfunc) PyEnumerate_Next,
    .tp_init = (initproc) PyEnumerate_Init,
    .tp_new = PyEnumerate_New,
};

typedef struct {
    PyObject_HEAD
    PyObject *iterable;
    PyObject *func;
} PyFilterObject;

static PyObject * PyFilter_New(PyTypeObject *type, PyObject *args, PyObject *kwargs){
    PyFilterObject *new_obj = (PyFilterObject *)type->tp_alloc(type, 0);
    if (!new_obj){
        return NULL;
    }
    return (PyObject *) new_obj;
}

int PyFilter_Init(PyFilterObject *self, PyObject *args, PyObject *kwargs){
    PyObject *seq, *func;
    if (!PyArg_ParseTuple(args, "OO", &func, &seq)){
        return -1;
    }
    if (!(self->iterable = PyObject_GetIter(seq))){
        std::string msg = "'%s' object is not iterable";
        PyObject *pystring = PyUnicode_FromFormat(msg.c_str(), seq->ob_type->tp_name);
        PyErr_SetObject(PyExc_TypeError, pystring);
        return -1;
    }
    if (PyObject_RichCompareBool(func, Py_None, Py_EQ)){
        PyObject *pymodule = PyImport_ImportModule("builtins");
        func = PyObject_GetAttrString(pymodule, "bool");
        Py_XDECREF(pymodule);
    }
    else {
        Py_XINCREF(func);
    }
    self->func = func;
    return 0;
}

static PyObject * PyFilter_Iter(PyFilterObject *self){
    Py_XINCREF(self);
    return (PyObject *)self;
}

static PyObject * PyFilter_Next(PyFilterObject *self){
    PyObject *item, *result;
    while (true){
        if (!(item = PyIter_Next(self->iterable))){
            if (PyErr_Occurred()){
                return NULL;
            }
            PyErr_SetString(PyExc_StopIteration, "");
            return NULL;
        }
        result = PyObject_CallOneArg(self->func, item);
        if (PyErr_Occurred()){
            return NULL;
        }
        else if(PyObject_IsTrue(result)){
            Py_XDECREF(result);
            return item;
        }
        Py_XDECREF(result);
    }
}

static void PyFilter_Dealloc(PyFilterObject *self){
    Py_XDECREF(self->iterable);
    Py_XDECREF(self->func);
    Py_TYPE(self)->tp_free((PyObject *) self);
}

static PyTypeObject PyFilterMyType {
    PyObject_HEAD_INIT(NULL)
    .tp_name = "cppbuiltins.filter",
    .tp_basicsize = sizeof(PyFilterObject),
    .tp_itemsize = 0,
    .tp_dealloc = (destructor) PyFilter_Dealloc,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_iter = (getiterfunc) PyFilter_Iter,
    .tp_iternext = (iternextfunc) PyFilter_Next,
    .tp_init = (initproc) PyFilter_Init,
    .tp_new = PyFilter_New,
};

static PyObject * c_getattr_func(PyObject *self, PyObject *args){
    PyObject *obj, *name, *objdefault = NULL, *result;
    if (!PyArg_ParseTuple(args, "OO|O", &obj, &name, &objdefault)){
        return NULL;
    }
    else if (!PyUnicode_Check(name)){
        char msg[] = "getattr(): attribute name must be string";
        PyErr_SetString(PyExc_TypeError, msg);
        return NULL;
    }
    result = PyObject_GetAttr(obj, name);
    if (PyErr_Occurred()){
        if (objdefault != NULL){
            if (PyErr_ExceptionMatches(PyExc_AttributeError)){
                PyErr_Clear();
                return objdefault;
            }
            return NULL;
        }
        return NULL;
    }
    return result;
}

static PyObject * c_hasattr_func(PyObject *self, PyObject *args){
    PyObject *obj, *name, *result;
    if (!PyArg_ParseTuple(args, "OO", &obj, &name)){
        return NULL;
    }
    else if (!PyUnicode_Check(name)){
        char msg[] = "hasattr(): attribute name must be string";
        PyErr_SetString(PyExc_TypeError, msg);
        return NULL;
    }
    result = PyObject_GetAttr(obj, name);
    if (PyErr_Occurred()){
        if (PyErr_ExceptionMatches(PyExc_AttributeError)){
            PyErr_Clear();
            Py_RETURN_FALSE;
        }
        return NULL;
    }
    Py_XDECREF(result);
    Py_RETURN_TRUE;
}

static PyObject * c_hasattr_api_func(PyObject *self, PyObject *args){
    PyObject *obj, *name;
    if (!PyArg_ParseTuple(args, "OO", &obj, &name)){
        return NULL;
    }
    else if (!PyUnicode_Check(name)){
        char msg[] = "hasattr(): attribute name must be string";
        PyErr_SetString(PyExc_TypeError, msg);
        return NULL;
    }
    if (PyObject_HasAttr(obj, name)){
        Py_RETURN_TRUE;
    }
    Py_RETURN_FALSE;
}

int Callable_Object(PyObject *item, const char name[]){
    PyObject * attr = PyObject_GetAttrString(item, name);
    if (PyCallable_Check(attr)){
        Py_XDECREF(attr);
        return 1;
    }
    Py_XDECREF(attr);
    return 0;
}

static PyObject * c_hash_func(PyObject *self, PyObject *item){
    if ((!PyObject_HasAttrString(item, "__hash__")) || !Callable_Object(item, "__hash__")){
        return PyErr_Format(PyExc_TypeError, "unhashable type: '%s'", item->ob_type->tp_name);
    }
    return PyObject_CallMethod(item, "__hash__", NULL);
}

static PyObject * c_hash_api_func(PyObject *self, PyObject *item){
    Py_hash_t hash_num;
    if ((hash_num = PyObject_Hash(item)) < 0){
        PyObject_HashNotImplemented(item);
        return NULL;
    }
    return PyLong_FromSsize_t(hash_num);
}

static PyObject * c_hex_func(PyObject *self, PyObject *args){
    signed long long num;
    if (!PyArg_ParseTuple(args, "L", &num)){
        return NULL;
    }
    std::string result = "";
    my_base_converter(&num, &result);
    return PyUnicode_FromString(result.c_str());
}

//METH_O
static PyObject * c_hex_api_func(PyObject *self, PyObject *num){
    PyObject *result, *temp = PyNumber_ToBase(num, 16);
    if (temp == NULL){
        return NULL;
    }
    result = PyObject_Str(temp);
    Py_XDECREF(temp);
    return result;
}

static PyObject * c_input_func(PyObject *self, PyObject *args){
    PyObject *pyprompt = NULL;
    std::string cprompt, temp;
    if (!PyArg_ParseTuple(args, "|O", &pyprompt)){
        return NULL;
    }
    if (pyprompt == NULL){
        cprompt = "";
    }
    else if (PyUnicode_CheckExact(pyprompt)){
        cprompt = (std::string)PyUnicode_AsUTF8(pyprompt);
    }
    else {
        pyprompt = PyObject_Repr(pyprompt);
        cprompt = (std::string)PyUnicode_AsUTF8(pyprompt);
        Py_XDECREF(pyprompt);
    }
    std::cout << cprompt.c_str();
    std::getline(std::cin, temp);
    return PyUnicode_FromString(temp.c_str());
}

static PyObject * c_isinstance_func(PyObject *self, PyObject *args){
    PyObject *item, *types;
    char msg[] = "isinstance() arg 2 must be a type or tuple of types";
    if (!PyArg_ParseTuple(args, "OO", &item, &types)){
        return NULL;
    }
    PyObject *item_bases = item->ob_type->tp_mro;
    if (PyType_Check(types)){
        if (PySequence_Contains(item_bases, types)){
            Py_RETURN_TRUE;
        }
        Py_RETURN_FALSE;
    }
    else if (PyTuple_Check(types)){
        types = PyObject_GetIter(types);
        PyObject *test_type;
        while ((test_type = PyIter_Next(types)) != NULL){
            if (!PyType_Check(test_type)){
                Py_XDECREF(types);
                Py_XDECREF(test_type);
                PyErr_SetString(PyExc_TypeError, msg);
                return NULL;
            }
            else if (PySequence_Contains(item_bases, test_type)){
                Py_XDECREF(types);
                Py_XDECREF(test_type);
                Py_RETURN_TRUE;
            }
        }
        Py_RETURN_FALSE;
    }
    PyErr_SetString(PyExc_TypeError, msg);
    return NULL;
}

PyObject * c_isinstance_api_func(PyObject *self, PyObject *args){
    PyObject *item, *types;
    if (!PyArg_ParseTuple(args, "OO", &item, &types)){
        return NULL;
    }
    int result = PyObject_IsInstance(item, types);
    if (result < 0){
        return NULL;
    }
    else if (result){
        Py_RETURN_TRUE;
    }
    Py_RETURN_FALSE;
}

static PyObject * c_issubclass_func(PyObject *self, PyObject *args){
    PyObject *item, *types;
    char msg[] = "issubclass() arg 2 must be a class or tuple of classes";
    if (!PyArg_ParseTuple(args, "OO", &item, &types)){
        return NULL;
    }
    if (!PyType_Check(item)){
        PyErr_SetString(PyExc_TypeError, "issubclass() arg 1 must be a class");
        return NULL;
    }
    PyObject *item_mro = PyObject_CallMethod(item, "mro", NULL);
    if (PyType_Check(types)){
        if (PySequence_Contains(item_mro, types)){
            Py_XDECREF(item_mro);
            Py_RETURN_TRUE;
        }
        Py_XDECREF(item_mro);
        Py_RETURN_FALSE;
    }
    else if (PyTuple_Check(types)){
        types = PyObject_GetIter(types);
        PyObject *test_type;
        while ((test_type = PyIter_Next(types)) != NULL){
            if (!PyType_Check(test_type)){
                Py_XDECREF(types);
                Py_XDECREF(test_type);
                Py_XDECREF(item_mro);
                PyErr_SetString(PyExc_TypeError, msg);
                return NULL;
            }
            else if (PySequence_Contains(item_mro, test_type)){
                Py_XDECREF(types);
                Py_XDECREF(test_type);
                Py_XDECREF(item_mro);
                Py_RETURN_TRUE;
            }
        }
        Py_XDECREF(item_mro);
        Py_RETURN_FALSE;
    }
    Py_XDECREF(item_mro);
    PyErr_SetString(PyExc_TypeError, msg);
    return NULL;
}

static PyObject * c_issubclass_api_func(PyObject *self, PyObject *args){
    PyObject *cls, *types;
    if (!PyArg_ParseTuple(args, "OO", &cls, &types)){
        return NULL;
    }
    int result = PyObject_IsSubclass(cls, types);
    if (result < 0){
        return NULL;
    }
    else if (result){
        Py_RETURN_TRUE;
    }
    Py_RETURN_FALSE;
}

typedef struct {
    PyObject_HEAD
    PyObject *seq_or_callable;
    PyObject *sentinel;
    Py_ssize_t index;
} PyIterObject;

static PyObject * PyIterObject_New(PyTypeObject *type, PyObject *seq, PyObject *sentinel){
    PyIterObject *new_obj = (PyIterObject *)type->tp_alloc(type, 0);
    if (!new_obj){
        return NULL;
    }
    return (PyObject *) new_obj;
}

int PyIterObject_Init(PyIterObject *self, PyObject *seq, PyObject *sentinel){
    if (!seq){
        PyErr_SetString(PyExc_TypeError, "iterator expects a sequence or callable!");
        return -1;
    }
    else if (sentinel && !PyCallable_Check(seq)){
        PyErr_SetString(PyExc_TypeError, "iterator(v, w): v must be callable");
        return -1;
    }
    self->seq_or_callable = seq;
    self->sentinel = sentinel;
    if (self->sentinel == NULL){
        self->index = 0;
    }
    return 0;
}

static PyObject * PyIterObject_Getitem(PyIterObject *self){
    PyObject *item = PySequence_GetItem(self->seq_or_callable, self->index);
    if (PyErr_Occurred() || item == NULL){
        if (PyErr_ExceptionMatches(PyExc_IndexError)){
            PyErr_Clear();
            PyErr_SetString(PyExc_StopIteration, "");
        }
        return NULL;
    }
    self->index += 1;
    return item;
}

static PyObject * PyIterObject_Callable(PyIterObject *self){
    PyObject *item = PyObject_CallNoArgs(self->seq_or_callable);
    if (PyErr_Occurred() || item == NULL || item == self->sentinel){
        if (PyErr_Occurred()){
            PyErr_Clear();
        }
        PyErr_SetString(PyExc_StopIteration, "");
        return NULL;
    }
    return item;
}

static PyObject * PyIterObject_Next(PyIterObject *self){
    PyObject *item;
    if (self->sentinel == NULL){
        item = PyIterObject_Getitem(self);
    }
    else {
        item = PyIterObject_Callable(self);
    }
    return item;
}

static PyObject * PyIterObject_Iter(PyIterObject *self){
    Py_XINCREF(self);
    self->index = 0;
    return (PyObject *)self;
}

static void PyIterObject_Dealloc(PyIterObject *self){
    Py_XDECREF(self->seq_or_callable);
    Py_XDECREF(self->sentinel);
    Py_TYPE(self)->tp_free((PyObject *)self);
}

static PyTypeObject PyIterObjectType = {
    PyObject_HEAD_INIT(NULL)
    .tp_name = "cppbuiltins.iterator",
    .tp_basicsize = sizeof(PyIterObject),
    .tp_itemsize = 0,
    .tp_dealloc = (destructor) PyIterObject_Dealloc,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_doc = "An iterator (for objects that don't have __iter__) in C++",
    .tp_iter = (getiterfunc) PyIterObject_Iter,
    .tp_iternext = (iternextfunc) PyIterObject_Next,
    .tp_init = (initproc) PyIterObject_Init,
    .tp_new = PyIterObject_New
};

static PyObject * c_iter_func(PyObject *self, PyObject *args){
    PyObject *seq_or_callable, *iterator, *sentinel = NULL;
    if (!PyArg_ParseTuple(args, "O|O", &seq_or_callable, &sentinel)){
        return NULL;
    }
    if (sentinel == NULL){
        iterator = PyObject_CallMethod(seq_or_callable, "__iter__", NULL);
        if (iterator != NULL){
            return iterator;
        }
        else {
            PyErr_Clear();
            if (!PyObject_HasAttrString(seq_or_callable, "__getitem__")){
                char msg[] = "'%s' object is not iterable";
                PyErr_Format(PyExc_TypeError, msg, seq_or_callable->ob_type->tp_name);
                return NULL;
            }
            else {
                PyObject *func = PyObject_GetAttrString(seq_or_callable, "__getitem__");
                if (!PyCallable_Check(func)){
                    Py_XDECREF(func);
                    char msg[] = "'%s' object is not iterable";
                    PyErr_Format(PyExc_TypeError, msg, seq_or_callable->ob_type->tp_name);
                    return NULL;
                }
                Py_XDECREF(func);
                Py_XINCREF(seq_or_callable);
                iterator = PyIterObject_New(&PyIterObjectType, seq_or_callable, NULL);
                if (PyIterObject_Init((PyIterObject *)iterator, seq_or_callable, NULL) < 0){
                    PyIterObject_Dealloc((PyIterObject *)iterator);
                    return NULL;
                }
                return iterator;
            }
        }
    }
    else {
        if (!PyCallable_Check(seq_or_callable)){
            PyErr_SetString(PyExc_TypeError, "iter(v, w): v must be callable");
            return NULL;
        }
        Py_XINCREF(seq_or_callable);
        Py_XINCREF(sentinel);
        iterator = PyIterObject_New(&PyIterObjectType, seq_or_callable, sentinel);
        if (PyIterObject_Init((PyIterObject *)iterator, seq_or_callable, sentinel) < 0){
            PyIterObject_Dealloc((PyIterObject *)iterator);
            return NULL;
        }
        return iterator;
    }
}

static PyObject * c_iter_api_func(PyObject *self, PyObject *args){
    PyObject *seq_or_callable, *sentinel = NULL;
    if (!PyArg_ParseTuple(args, "O|O", &seq_or_callable, &sentinel)){
        return NULL;
    }
    else if (sentinel != NULL){
        if (!PyCallable_Check(seq_or_callable)){
            PyErr_SetString(PyExc_TypeError, "iter(v, w): v must be callable");
            return NULL; 
        }
        return PyCallIter_New(seq_or_callable, sentinel);
    }
    return PyObject_GetIter(seq_or_callable);
}

//METH_O
static PyObject * c_len_func(PyObject *self, PyObject *item){
    PyObject *result;
    if (PyObject_HasAttrString(item, "__len__")){
        item = PyObject_CallMethod(item, "__len__", NULL);
        Py_ssize_t length;
        if (item == NULL){
            return NULL;
        }
        else if (PyLong_CheckExact(item)){
            length = PyLong_AsSsize_t(item);
            if (length < 0){
                if (!PyErr_Occurred()){
                    PyErr_SetString(PyExc_ValueError, "__len__() should return >= 0");
                }
                return NULL;
            }
            return item;
        }
        else if ((result = PyNumber_Index(item)) != NULL){
            length = PyLong_AsSsize_t(result);
            if (length < 0){
                if (!PyErr_Occurred()){
                    PyErr_SetString(PyExc_ValueError, "__len__() should return >= 0");
                }
                return NULL;
            }
            Py_XDECREF(item);
            return result;
        }
        else {
            if (PyErr_Occurred()){
                return NULL;
            }
            char msg[] = "'%s' object cannot be interpreted as an integer";
            PyErr_Format(PyExc_TypeError, msg, item->ob_type->tp_name);
        }
    }
    char msg[] = "object of type '%s' has no len()";
    PyErr_Format(PyExc_TypeError, msg, item->ob_type->tp_name);
    return NULL;
}

static PyObject * c_len_api_func(PyObject *self, PyObject *item){
    Py_ssize_t result = PyObject_Length(item);
    if (result < 0){
        if (!PyErr_Occurred()){
            PyErr_SetString(PyExc_ValueError, "__len__() should return >= 0");
        }
        return NULL;
    }
    return PyLong_FromSsize_t(result);
}

typedef struct {
    PyObject_HEAD
    PyObject *func;
    PyObject *iters;
    Py_ssize_t nargs;
} PyMapObject;

void PyMapError(PyObject *iters, PyObject *seq, PyObject *iterargs, Py_ssize_t count);

static PyObject * PyMapObject_New(PyTypeObject *type, PyObject *args, PyObject *kwargs){
    PyMapObject *new_obj = (PyMapObject *)type->tp_alloc(type, 0);
    if (!new_obj){
        return NULL;
    }
    Py_ssize_t count, numargs = PyTuple_Size(args);
    PyObject *iters, *seq, *iterseq, *newargs, *newargsiter;
    if (numargs < 2){
        PyErr_SetString(PyExc_TypeError, "map() must have at least two arguments.");
        return NULL;
    }
    else if (!PyCallable_Check(PyTuple_GET_ITEM(args, 0))){
        char msg[] = "'%s' object is not callable";
        PyErr_Format(PyExc_TypeError, msg, PyTuple_GET_ITEM(args, 0)->ob_type->tp_name);
        return NULL;
    }
    newargs = PyTuple_GetSlice(args, 1, numargs);
    newargsiter = PyObject_GetIter(newargs);
    Py_XDECREF(newargs);
    new_obj->nargs = numargs - 1;
    iters = PyTuple_New(numargs - 1);
    count = 0;
    while ((seq = PyIter_Next(newargsiter)) != NULL){
        if (!PyObject_HasAttrString(seq, "__iter__") && !PyObject_HasAttrString(seq, "__getitem__")){
            PyMapError(iters, seq, newargsiter, count);
            return NULL;
        }
        else if ((iterseq = PyObject_GetIter(seq)) == NULL){
            PyMapError(iters, seq, newargsiter, count);
            return NULL;
        }
        Py_XDECREF(seq);
        PyTuple_SET_ITEM(iters, count, iterseq);
        count += 1;
    }
    Py_XDECREF(newargsiter);
    new_obj->iters = iters;
    new_obj->func = PySequence_ITEM(args, 0);
    return (PyObject *) new_obj;
}

void PyMapError(PyObject *iters, PyObject *seq, PyObject *iterargs, Py_ssize_t count){
    for (Py_ssize_t i = 0; i < count; i++){
        Py_XDECREF(PyTuple_GET_ITEM(iters, i));
    }
    PyErr_Clear();
    Py_XDECREF(iters);
    char msg[] = "'%s' object is not iterable";
    PyErr_Format(PyExc_TypeError, msg, seq->ob_type->tp_name);
    Py_XDECREF(seq);
    Py_XDECREF(iterargs);
}

static PyObject * PyMapObject_Iter(PyMapObject *self){
    Py_XINCREF(self);
    return (PyObject *)self;
}

static PyObject * PyMapObject_Next(PyMapObject *self){
    PyThreadState *tstate = PyThreadState_Get();
    PyObject *val;
    PyObject *arg_stack[self->nargs];
    for (Py_ssize_t i = 0; i < self->nargs; i++){
        val = PyIter_Next(PyTuple_GET_ITEM(self->iters, i));
        if (val == NULL){
            for (Py_ssize_t c = 0; c < i; c++){
                Py_XDECREF(arg_stack[c]);
            }
            PyErr_SetString(PyExc_StopIteration, "");
            return NULL;
        }
        arg_stack[i] = val;
    }
    return _PyObject_VectorcallTstate(tstate, self->func, arg_stack, self->nargs, NULL);
}

static void PyMapObject_Dealloc(PyMapObject *self){
    Py_XDECREF(self->func);
    for (Py_ssize_t i = 0; i < self->nargs; i++){
        Py_XDECREF(PyTuple_GET_ITEM(self->iters, i));
    }
    Py_XDECREF(self->iters);
    Py_TYPE(self)->tp_free((PyObject *)self);
}

static PyTypeObject PyMapObjectType = {
    PyObject_HEAD_INIT(NULL)
    .tp_name = "cppbuiltins.map",
    .tp_basicsize = sizeof(PyMapObject),
    .tp_itemsize = 0,
    .tp_dealloc = (destructor) PyMapObject_Dealloc,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_doc = "An map object in C++",
    .tp_iter = (getiterfunc) PyMapObject_Iter,
    .tp_iternext = (iternextfunc) PyMapObject_Next,
    .tp_new = PyMapObject_New
};

static void insertion_sort(PyObject *list, Py_ssize_t *leftp, Py_ssize_t *rightp){
    Py_ssize_t right, indexpos, left = 0;
    Py_ssize_t size = PyList_Size(list);
    PyObject *item, *tempitem;
    if (leftp != NULL){
        left = *leftp;
    }
    if (rightp == NULL){
        right = size - 1;
    }
    else {
        right = *rightp;
    }
    if (left < 0 || left >= right){
        PyErr_SetString(PyExc_IndexError, "left side out of bounds");
        return;
    }
    else if (right <= left || right >= size){
        PyErr_SetString(PyExc_IndexError, "right side out of bounds");
        return;
    }
    for (Py_ssize_t pos = left + 1; pos < right + 1; pos++){
        item = PyList_GetItem(list, pos);
        Py_XINCREF(item);
        indexpos = pos;
        while ((indexpos > left) &&
                PyObject_RichCompareBool(PyList_GetItem(list, indexpos - 1), item, Py_GT)){
            tempitem = PyList_GetItem(list, indexpos - 1);
            Py_XINCREF(tempitem);
            Py_XINCREF(PyList_GetItem(list, indexpos));
            if (PyList_SetItem(list, indexpos, tempitem) == -1){
                return;
            }
            indexpos--;
        }
        Py_XINCREF(PyList_GetItem(list, indexpos));
        if (PyList_SetItem(list, indexpos, item) == -1){
            return;
        }
    }
}

static PyObject * c_insertion_sort_func(PyObject *self, PyObject *list){
    if (!PyObject_IsInstance(list, (PyObject *)&PyList_Type)){
        char msg[] = "insertion sort object type must be list, not '%s'";
        PyErr_Format(PyExc_TypeError, msg, list->ob_type->tp_name);
        return NULL;
    }
    Py_ssize_t start = 0;
    insertion_sort(list, &start, NULL);
    if (PyErr_Occurred()){
        return NULL;
    }
    Py_RETURN_NONE;
}

static PyObject * merge(PyObject *left, PyObject *right){
    if (left == NULL || right == NULL){
        return NULL;
    }
    Py_ssize_t rightsize = PyList_Size(right);
    Py_ssize_t leftsize = PyList_Size(left);
    if (!leftsize){
        return right;
    }
    else if (!rightsize){
        return left;
    }
    PyObject *result = PyList_New(0);
    PyObject *leftitem, *rightitem, *itemlist;
    Py_ssize_t lpos = 0;
    Py_ssize_t rpos = 0;
    Py_ssize_t totalsize = 0;
    while (PyList_Size(result) < PyList_Size(left) + PyList_Size(right)){
        leftitem = PyList_GetItem(left, lpos);
        rightitem = PyList_GetItem(right, rpos);
        if (PyObject_RichCompareBool(leftitem, rightitem, Py_LE)){
            Py_XINCREF(leftitem);
            if (PyList_Append(result, leftitem) == -1){
                Py_XDECREF(result);
                return NULL;
            }
            lpos += 1;
        }
        else {
            Py_XINCREF(rightitem);
            if (PyList_Append(result, rightitem) == -1){
                Py_XDECREF(result);
                return NULL;
            }
            rpos += 1;
        }
        if (lpos == leftsize){
            if ((itemlist = PyList_GetSlice(right, rpos, rightsize)) == NULL){
                Py_XDECREF(result);
                return NULL;
            }
            totalsize = PyList_Size(result);
            if (PyList_SetSlice(result, totalsize, totalsize + PyList_Size(itemlist), itemlist) < 0){
                Py_XDECREF(itemlist);
                Py_XDECREF(result);
                return NULL;
            }
            break;
        }
        else if (rpos == rightsize){
            if ((itemlist = PyList_GetSlice(left, lpos, leftsize)) == NULL){
                Py_XDECREF(result);
                return NULL;
            }
            totalsize = PyList_Size(result);
            if (PyList_SetSlice(result, totalsize, totalsize + PyList_Size(itemlist), itemlist) < 0){
                Py_XDECREF(itemlist);
                Py_XDECREF(result);
                return NULL;
            }
            break;
        }
    }
    return result;
}

static PyObject * merge_sort(PyObject *list){
    Py_ssize_t end = PyList_Size(list);
    if (end < 2){
        return list;
    }
    Py_ssize_t middle = end / 2;
    PyObject *left = PyList_GetSlice(list, 0, middle);
    PyObject *right = PyList_GetSlice(list, middle, end);
    if (left == NULL || right == NULL){
        return NULL;
    }
    PyObject *result = merge(merge_sort(left), merge_sort(right));
    Py_XDECREF(left);
    Py_XDECREF(right);
    return result;
}

static PyObject * c_merge_sort_func(PyObject *self, PyObject *list){
    PyObject *result, *listcopy;
    if (!PyObject_IsInstance(list, (PyObject *)&PyList_Type)){
        listcopy = PySequence_List(list);
        if (listcopy == NULL){
            char msg[] = "merge sort object type must be an iterable, not '%s'";
            PyErr_Format(PyExc_TypeError, msg, list->ob_type->tp_name);
            return NULL;
        }
    }
    else {
        listcopy = PyObject_CallMethod(list, "copy", NULL);
        if (listcopy == NULL){
            PyErr_SetString(PyExc_TypeError, "Unable to copy list");
            return NULL;
        }
    }
    result = merge_sort(listcopy);
    Py_XDECREF(listcopy);
    if (PyErr_Occurred()){
        return NULL;
    }
    return result;
}

static Py_ssize_t calc_min_run(Py_ssize_t length){
    Py_ssize_t MINRUN = 32, r = 0;
    while (length >= MINRUN){
        r |= length & 1;
        length >>= 1;
    }
    return length + r;
}

static PyObject * timsort(PyObject *list){
    Py_ssize_t n = PyList_Size(list);
    Py_ssize_t min_run = calc_min_run(n);
    Py_ssize_t end, size, right;
    PyObject *arrayslice, *templist;
    for (Py_ssize_t start = 0; start < n; start += min_run){
        end = Py_MIN(start + min_run - 1, n - 1);
        insertion_sort(list, &start, &end);
    }
    size = min_run;
    while (size < n){
        for (Py_ssize_t left = 0; left < n; left += 2*size){
            right = Py_MIN(left + 2*size - 1, n - 1);
            arrayslice = PyList_GetSlice(list, left, right);
            templist = merge_sort(arrayslice);
            Py_XDECREF(arrayslice);
            if (PyList_SetSlice(list, left, right, templist) == -1){
                Py_XDECREF(templist);
                PyList_Type.tp_clear(list);
                Py_XDECREF(list);
                return NULL;
            }
        }
        size *= 2;
    }
    return list;
}

static PyObject * c_timesort_func(PyObject *self, PyObject *list){
    PyObject *result;
    if (!PyObject_IsInstance(list, (PyObject *)&PyList_Type)){
        result = PySequence_List(list);
        if (result == NULL){
            char msg[] = "merge sort object type must be an iterable, not '%s'";
            PyErr_Format(PyExc_TypeError, msg, list->ob_type->tp_name);
            return NULL;
        }
    }
    else {
        result = list;
    }
    result = timsort(result);
    if (PyErr_Occurred() || result == NULL){
        return NULL;
    }
    //return result;
    Py_RETURN_NONE;
}

/*
static PyObject * dummy_id(PyObject *item){
    return item;
}*/

static PyObject * min_max_loop(PyObject *args, PyObject *kwargs, int op){
    PyObject *iterable, *temp, *defaultval = NULL, *key = NULL;
    PyObject *item, *value, *topitem, *maxvalue;
    int cmp, positional = PyTuple_Size(args) > 1;
    const char *name = op == Py_GT ? "max" : "min";
    char *kwlist[] = {(char *)"key", (char *)"default", NULL};
    if (positional){
        iterable = args;
    }
    else if (!PyArg_ParseTuple(args, "O", &iterable)){
        PyErr_Format(PyExc_TypeError, "%s expects at least 1 argument, recieved 0", name);
        return NULL;
    }
    temp = PyTuple_New(0);
    if (!PyArg_ParseTupleAndKeywords(temp, kwargs, (op == Py_GT) ? "|$OO:max":"|$OO:min",
                                     kwlist, &key, &defaultval)){
        Py_XDECREF(temp);
        return NULL;
    }
    Py_XDECREF(temp);
    if (positional && defaultval != NULL){
        char msg[] = "Cannot specify a default for max() with multiple positional arguments";
        PyErr_SetString(PyExc_TypeError, msg);
        return NULL;
    }
    //PyObject * (*func)(PyObject *) = &dummy_id; function pointer
    iterable = PyObject_GetIter(iterable);
    if (iterable == NULL || PyErr_Occurred()){
        if (PyErr_Occurred()){
            PyErr_Clear();
        }
        PyErr_Format(PyExc_TypeError, "%s argument was not iterable", name);
        return NULL;
    }
    if ((item = PyIter_Next(iterable)) == NULL){
        Py_XDECREF(iterable);
        if (PyErr_Occurred()){
            if (PyErr_ExceptionMatches(PyExc_StopIteration)){
                PyErr_Clear();
                Py_XINCREF(defaultval);
                return defaultval;
            }
            return NULL;
        }
        Py_XINCREF(defaultval);
        return defaultval;
    }
    topitem = item;
    if (key != NULL){
        if ((value = PyObject_CallOneArg(key, item)) == NULL){
            Py_XDECREF(item);
            return NULL;
        }
        maxvalue = value;
    }
    else {
        maxvalue = value = topitem = item;
    }
    Py_XINCREF(value);
    while ((item = PyIter_Next(iterable)) != NULL){
        if (key != NULL){
            if ((value = PyObject_CallOneArg(key, item)) == NULL){
                Py_XDECREF(item);
                Py_XDECREF(topitem);
                Py_XDECREF(maxvalue);
                return NULL;
            }
        }
        else {
            value = item;
            Py_XINCREF(value);
        }
        cmp = PyObject_RichCompareBool(value, maxvalue, op); // Py_GT == value > maxvalue
        if (cmp < 0){
            Py_XDECREF(item);
            Py_XDECREF(value);
            Py_XDECREF(topitem);
            Py_XDECREF(maxvalue);
            return NULL;
        }
        else if (cmp){
            Py_XDECREF(topitem);
            Py_XDECREF(maxvalue);
            topitem = item;
            maxvalue = value;
        }
        else {
            Py_XDECREF(item);
            Py_XDECREF(value);
        }
    }
    Py_XDECREF(maxvalue);
    return topitem;
}

static PyObject * c_max_func(PyObject *self, PyObject *args, PyObject *kwargs){
    return min_max_loop(args, kwargs, Py_GT);
}

static PyObject * c_min_func(PyObject *self, PyObject *args, PyObject *kwargs){
    PyObject * result = min_max_loop(args, kwargs, Py_LT);
    return result;
}

static inline PyObject * CLEAR_AND_RETURN(PyObject *item){
    if (item != NULL){
        PyErr_Clear();
    }
    return item;
}

static PyObject * c_next_func(PyObject *self, PyObject *args){
    PyObject *item, *iterator, *sentinel = NULL;
    if (!PyArg_ParseTuple(args, "O|O", &iterator, &sentinel)){
        return NULL;
    }
    if (PyObject_HasAttrString(iterator, "__next__")){
        if ((item = PyObject_CallMethod(iterator, "__next__", NULL)) == NULL){
            return PyErr_ExceptionMatches(PyExc_StopIteration) ? CLEAR_AND_RETURN(sentinel) : NULL;
        }
        return item;
    }
    PyErr_Format(PyExc_TypeError, "'%s' object is not an iterator", iterator->ob_type->tp_name);
    return NULL;
}

static PyObject * c_next_api_func(PyObject *self, PyObject *args){
    PyObject *item, *iterator, *sentinel = NULL;
    if (!PyArg_ParseTuple(args, "O|O", &iterator, &sentinel)){
        return NULL;
    }
    if (!PyIter_Check(iterator)){
        PyErr_Format(PyExc_TypeError, "'%s' object is not an iterator", iterator->ob_type->tp_name);
        return NULL;
    }
    else if ((item = PyIter_Next(iterator)) == NULL){
        if (!PyErr_Occurred()){
            PyErr_SetNone(PyExc_StopIteration);
        }
        return PyErr_ExceptionMatches(PyExc_StopIteration) ? CLEAR_AND_RETURN(sentinel) : NULL;
    }
    return item;
}

static PyObject * c_oct_func(PyObject *self, PyObject *args){
    signed long long num;
    if (!PyArg_ParseTuple(args, "L", &num)){
        return NULL;
    }
    std::string result = "";
    my_base_converter(&num, 'O', &result);
    return PyUnicode_FromString(result.c_str());
}

//METH_O
static PyObject * c_oct_api_func(PyObject *self, PyObject *num){
    PyObject *result, *temp = PyNumber_ToBase(num, 8);
    if (temp == NULL){
        return NULL;
    }
    result = PyObject_Str(temp);
    Py_XDECREF(temp);
    return result;
}

static inline const char *type_name(PyObject *item){
    return item->ob_type->tp_name;
}

typedef struct NumStruct {
    unsigned long long int_num;
    long double float_num;
    short int int_success;
    short int float_success;
} NumStruct;

static void parse_num(PyObject *num, NumStruct *nstruct){
    nstruct->float_success = 0;
    nstruct->int_success = 0;
    if (PyLong_Check(num)){
        nstruct->int_num = PyLong_AsUnsignedLongLong(num);
        nstruct->int_success = 1;
    }
    else if (PyFloat_Check(num)){
        nstruct->float_num = PyFloat_AsDouble(num);
        nstruct->float_success = 1;
    }  
}

static PyObject * build_pow_value(signed long long num){
    return PyLong_FromLongLong(num);
}

static PyObject * build_pow_value(long double num){
    return PyFloat_FromDouble(num);
}

static PyObject * c_pow_func(PyObject *self, PyObject *args, PyObject *kwargs){
    PyObject *tempbase, *tempexp, *tempmod = NULL;
    char *kwlist[] = {(char *)"base", (char *)"exp", (char *)"mod", NULL};
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "OO|O", kwlist, &tempbase, &tempexp, &tempmod)){
        return NULL;
    }
    short int have_mod = tempmod != NULL;
    char typeerro1[] = "unsupported operand type(s) for pow(): '%s', '%s', '%s'";
    char typerror2[] = "unsupported operand type(s) for ** or pow(): '%s' and '%s'";
    char type_msg[65];
    have_mod ? memcpy(type_msg,typeerro1,strlen(typeerro1)+1) : memcpy(type_msg,typerror2,strlen(typerror2)+1);
    NumStruct nstruct;
    parse_num(tempbase, &nstruct);
    if (!(nstruct.float_success || nstruct.int_success)){
        if (have_mod){
            PyErr_Format(PyExc_TypeError, type_msg, type_name(tempbase), type_name(tempexp), type_name(tempmod));
        }
        else{
            PyErr_Format(PyExc_TypeError, type_msg, type_name(tempbase), type_name(tempexp));
        }
        return NULL;
    }
    auto basenum = nstruct.int_success ? nstruct.int_num : nstruct.float_num;
    parse_num(tempexp, &nstruct); 
    if (!(nstruct.float_success || nstruct.int_success)){
        if (have_mod){
            PyErr_Format(PyExc_TypeError, type_msg, type_name(tempbase), type_name(tempexp), type_name(tempmod));
        }
        else{
            PyErr_Format(PyExc_TypeError, type_msg, type_name(tempbase), type_name(tempexp));
        }
        return NULL;
    }
    auto expnum = nstruct.int_success ? nstruct.int_num : nstruct.float_num;
    auto result = pow(basenum, expnum);
    if (!have_mod){
        if (!PyLong_Check(tempbase) || !PyLong_Check(tempexp)){
            return build_pow_value(result);
        }
        return build_pow_value((signed long long)result);
    }
    parse_num(tempmod, &nstruct);
    auto modnum = nstruct.int_success ? nstruct.int_num : nstruct.float_num;
    if (!(nstruct.float_success||nstruct.int_success)){
        PyErr_Format(PyExc_TypeError, type_msg, type_name(tempbase), type_name(tempexp), type_name(tempmod));
        return NULL;
    }
    if (!(basenum == trunc(basenum) && expnum == trunc(expnum) && modnum == trunc(modnum))){
        PyErr_SetString(PyExc_TypeError, "pow() 3rd argument not allowed unless all arguments are integers");
        return NULL;
    }
    else if (modnum == 0){
        PyErr_SetString(PyExc_TypeError, "pow() 3rd argument cannot be 0");
        return NULL;
    }
    return build_pow_value((signed long long)result % (Py_ssize_t)modnum);
}

static PyObject * c_pow_api_func(PyObject *self, PyObject *args, PyObject *kwargs){
    PyObject *basenum, *expnum, *modnum = NULL;
    char *kwlist[] = {(char *)"base", (char *)"exp", (char *)"mod", NULL};
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "OO|O", kwlist, &basenum, &expnum, &modnum)){
        return NULL;
    }
    return modnum == NULL ? PyNumber_Power(basenum, expnum, Py_None) : PyNumber_Power(basenum, expnum, modnum);
}

/*typedef struct {
    PyObject_HEAD
    llong index;
    PyObject *seq;
} PyReverseObject;*/

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
    {"divmod", c_divmod_func, METH_VARARGS, "divmod() in C++"},
    {"divmod_api", c_divmod_api_func, METH_VARARGS, "divmod() using C API."},
    {"getattr", c_getattr_func, METH_VARARGS, "getattr() using C++"},
    {"hasattr", c_hasattr_func, METH_VARARGS, "hasattr() using C++"},
    {"hasattr_api", c_hasattr_api_func, METH_VARARGS, "hasattr() using C API."},
    {"hash", c_hash_func, METH_O, "hash() using C++"},
    {"hash_api", c_hash_api_func, METH_O, "hash() using C API."},
    {"hex", c_hex_func, METH_VARARGS, "hex() in C++"},
    {"hex_api", c_hex_api_func, METH_O, "hex() using C API."},
    {"input", c_input_func, METH_VARARGS, "input() in C++"},
    {"isinstance", c_isinstance_func, METH_VARARGS, "isinstance() in C++"},
    {"isinstance_api", c_isinstance_api_func, METH_VARARGS, "isinstance() in C API."},
    {"issubclass", c_issubclass_func, METH_VARARGS, "issubclass() in C++"},
    {"issubclass_api", c_issubclass_api_func, METH_VARARGS, "issubclass() using C API."},
    {"iter", c_iter_func, METH_VARARGS, "iter() in C++"},
    {"iter_api", c_iter_api_func, METH_VARARGS, "iter() using C API."},
    {"len", c_len_func, METH_O, "len() in C++."},
    {"len_api", c_len_api_func, METH_O, "len() using C API."},
    {"insertion_sort", c_insertion_sort_func, METH_O, "insertion sort in C++."},
    {"merge_sort", c_merge_sort_func, METH_O, "merge sort in C++."},
    {"timsort", c_timesort_func, METH_O, "timsort in C++."},
    {"max", PyKwargFunction c_max_func, METH_VARARGS | METH_KEYWORDS, "max() in C++."},
    {"min", PyKwargFunction c_min_func, METH_VARARGS | METH_KEYWORDS, "min() in C++."},
    {"next", c_next_func, METH_VARARGS, "next() in C++."},
    {"next_api", c_next_api_func, METH_VARARGS, "next() using C API."},
    {"oct", c_oct_func, METH_VARARGS, "oct() in C++."},
    {"oct_api", c_oct_api_func, METH_O, "oct() using C API."},
    {"pow", PyKwargFunction c_pow_func, METH_VARARGS | METH_KEYWORDS, "pow() in C++."},
    {"pow_api", PyKwargFunction c_pow_api_func, METH_VARARGS | METH_KEYWORDS, "pow() in C API."},
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

int Add_PyType_Func(PyObject *pymodule, PyTypeObject *mytype);

//Module initialization function
PyMODINIT_FUNC
PyInit_cppbuiltins(void) {
    /*std::vector<PyTypeObject> MyPyTypeList;
    MyPyTypeList.push_back(PyEnumerate_Type);*/
    PyObject* m = PyModule_Create(&cppbuiltinsmodule);
    if (m == NULL) {
        return NULL;
    }
    /*if (PyType_Ready(&PyEnumerate_Type) < 0){
        std::printf("Failure\n");
        return NULL;
    }
    Py_INCREF(&PyEnumerate_Type);
    if (PyModule_AddObject(m, "enumerate", (PyObject *) &PyEnumerate_Type) < 0){
        Py_DECREF(&PyEnumerate_Type);
        Py_DECREF(m);
        return NULL;
    }*/
    Add_PyType_Func(m, &MyClassMethod_Type);
    Add_PyType_Func(m, &PyEnumerate_Type);
    Add_PyType_Func(m, &PyFilterMyType);
    Add_PyType_Func(m, &PyIterObjectType);
    Add_PyType_Func(m, &PyMapObjectType);
    return m;
}

int Add_PyType_Func(PyObject *pymodule, PyTypeObject *mytype){
    std::string name = mytype->tp_name;
    name = name.substr(name.find('.')+1);
    //std::printf("%s\n", name.c_str());
    if (PyType_Ready(mytype) < 0){
        std::printf("Failure\n");
        return 0;
    }
    Py_INCREF(mytype);
    if (PyModule_AddObject(pymodule, name.c_str(), (PyObject *) mytype) < 0){
        Py_DECREF(mytype);
        Py_DECREF(pymodule);
        return 0;
    }
    return 1;
}
