#include <stdlib.h>
#include <stdint.h>
#include "str_str_wyhash.h"
#include <stdbool.h>
#include <inttypes.h>
#include "flags.h"

#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include "structmember.h"


typedef struct
{
    PyObject_HEAD;
    h_t* ht;    
    i_t iter_idx;
    i_t iter_num;
} iterObj;

typedef struct {
    PyObject_HEAD;
    h_t* ht;
    bool valid_ht;
    i_t iter_idx;
    i_t iter_num;
    kbox_t temp_key;
    vbox_t temp_val;
    bool temp_isvalid;
    iterObj* value_iterator;
    iterObj* item_iterator;
    char key_size_str[6];
    char val_size_str[6];
    uint32_t flags;
} dictObj;


static PyObject* value_iter(iterObj* self);
static PyObject* value_iternext(iterObj* self);
static PyObject* item_iter(iterObj* self);
static PyObject* item_iternext(iterObj* self);


static PyTypeObject valueIterType_str_str = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "str->str value iterator",
    .tp_doc = "",
    .tp_basicsize = sizeof(iterObj),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_new = PyType_GenericNew,
    .tp_iter = (getiterfunc) value_iter,
    .tp_iternext = (iternextfunc) value_iternext,
};

static PyTypeObject itemIterType_str_str = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "str->str item iterator",
    .tp_doc = "",
    .tp_basicsize = sizeof(iterObj),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_new = PyType_GenericNew,
    .tp_iter = (getiterfunc) item_iter,
    .tp_iternext = (iternextfunc) item_iternext,
};



static PyObject* value_iter(iterObj* self) {
    /*
    Returns an iterator for iterating over the dictionary values. This function is invoked when dict.values() method is called.
    */

    Py_INCREF(self);
    self->iter_idx = 0;
    self->iter_num = 0;
    return (PyObject *) self;    
}


static PyObject* value_iternext(iterObj* self) {
    /*
    Iterates over the values when __next__ is called on the iterator. Each time this function is called by __next__, the next value is returned.
    */

    if (self->iter_num >= self->ht->size) {
        PyErr_SetNone(PyExc_StopIteration);
        return NULL;
    }

    h_t* h = self->ht;
    vbox_t val;
    int v_step_inc = h->v_step_increment;

    for (i_t i=self->iter_idx; ; ++i) {
        if (!_flags_isempty(h->flags, i)) {
            val = _get_val(h, GET_PTR(i, v_step_inc));
            self->iter_idx = i+1;
            self->iter_num += 1;
            break;
        }
    }

    return PyUnicode_DecodeUTF8(val.str, val.len, NULL);
}


static PyObject* item_iter(iterObj* self) {
    /*
    Returns an iterator for iterating over the dictionary items. This function is invoked when dict.items() method is called.
    */

    Py_INCREF(self);
    self->iter_idx = 0;
    self->iter_num = 0;
    return (PyObject *) self;    
}


static PyObject* item_iternext(iterObj* self) {
    /*
    Iterates over the items when __next__ is called on the iterator. Each time this function is called by __next__, the next item (key, value) is returned.
    */

    if (self->iter_num >= self->ht->size) {
        PyErr_SetNone(PyExc_StopIteration);
        return NULL;
    }

    h_t* h = self->ht;
    kbox_t key; vbox_t val;
    int k_step_inc = h->k_step_increment;
    int v_step_inc = h->v_step_increment;

    for (i_t i=self->iter_idx; ; ++i) {
        if (!_flags_isempty(h->flags, i)) {
            key = _get_key(h, GET_PTR(i, k_step_inc));
            val = _get_val(h, GET_PTR(i, v_step_inc));
            self->iter_idx = i+1;
            self->iter_num += 1;
            break;
        }
    }

    return PyTuple_Pack(2, PyUnicode_DecodeUTF8(key.str, key.len, NULL), PyUnicode_DecodeUTF8(val.str, val.len, NULL));
}

void _set_default_flags(dictObj* self) {
    self->flags = 0;
    _set_flag(&self->flags, FLAG_POP_ARG_EXC, true);
    _set_flag(&self->flags, FLAG_POP_RET_EXC, true);
}

void _destroy(dictObj* self){
    /*
    Called by the destructor for deleting the hashtable.
    */
 
    if (self->valid_ht == true){
        mdict_delete_ht(self->ht);
        self->valid_ht = false;
    }
}

void _create(dictObj* self, i_t k_maxLength, i_t v_maxLength){
    /*
    Called by the constructor for allocating and initializing the hashtable.
    */
 
    if (self->valid_ht == false){
        ht_param param = {5, k_maxLength, 5, v_maxLength, k_maxLength+2, v_maxLength+2};
        self->ht = mdict_create(&param);  
        self->valid_ht = true;
        self->iter_idx = 0;
        self->iter_num = 0;
        self->temp_isvalid = false;
        sprintf(self->key_size_str, "%d", self->ht->key_str_len);
        sprintf(self->val_size_str, "%d", self->ht->val_str_len);
    }    
}

 
static void custom_dealloc(dictObj* self) {
    /*
    The destructor
    */

    _destroy(self);
    Py_TYPE(self)->tp_free((PyObject *) self);
}

static PyObject* custom_new(PyTypeObject *type, PyObject *args) {
    /*
    Allocates the dictObj
    */

    dictObj* self = (dictObj*) type->tp_alloc(type, 0);
    self->valid_ht = false;
    self->ht = NULL;
    _set_default_flags(self);
    return (PyObject*) self;
}


static int custom_init(dictObj* self, PyObject *args) {
    /*
    Constructor for allocating and initializing the hashtable along with the iterators.
    */

    int k_maxLength, v_maxLength;

    if (!PyArg_ParseTuple(args, "ii", &k_maxLength, &v_maxLength)) {
        Py_DECREF(self);
        return -1;
    }

    if (k_maxLength > str_len_MAX || v_maxLength > str_len_MAX) {
        PyErr_SetString(PyExc_ValueError, "Both key_maxLength and value_maxLength must be at most 65535");
        Py_DECREF(self);
        return -1;
    }

    _create(self, k_maxLength, v_maxLength);

    self->value_iterator = (iterObj *) valueIterType_str_str.tp_alloc(&valueIterType_str_str, 0); 
    self->value_iterator->ht = self->ht;
    self->value_iterator->iter_idx = 0;
    self->value_iterator->iter_num = 0;

    self->item_iterator = (iterObj *) itemIterType_str_str.tp_alloc(&itemIterType_str_str, 0); 
    self->item_iterator->ht = self->ht;
    self->item_iterator->iter_idx = 0;
    self->item_iterator->iter_num = 0;

    return 0;
}


static PyObject* del(dictObj* self, PyObject* args){
    /*
    dict.pop() invokes this function. Only accepts a python string object of size at most key_str_len.
    If the key argument is present, then this function deletes it. Otherwise it will either raise a KeyError
    or return a None object depending on whether FLAG_POP_RET_EXC flag is set or not. 
    By default, FLAG_POP_RET_EXC flag is set. If FLAG_POP_RET_EXC is set, then it raises an exception if the key
    has length larger than key_str_len, otherwise returns None. By default, FLAG_POP_ARG_EXC is set.
    */

    kbox_t k; vbox_t v; int ret_val;
    int len;
    PyObject* str_obj;

    if (!PyArg_ParseTuple(args, "O", &str_obj)){
        return NULL;
    }

    k.str = PyUnicode_AsUTF8AndSize(str_obj, &len);
    if (k.str == NULL)
        return NULL;

    if (len > self->ht->key_str_len) {
        if (!_get_flag(self->flags, FLAG_POP_ARG_EXC))
            return Py_BuildValue("");
        char msg[40];
        sprintf(msg, "key length must be at most %d", self->ht->key_str_len);
        PyErr_SetString(PyExc_TypeError, msg);
        return NULL;
    }
    k.len = len;

    if (self->temp_isvalid && (k.len == self->temp_key.len) && _strncmp(k.str, self->temp_key.str, k.len))
        self->temp_isvalid = false;


    if (mdict_del_map(self->ht, k, &v) == -2) {
        if (!_get_flag(self->flags, FLAG_POP_RET_EXC))
            return Py_BuildValue("");
        PyErr_SetString(PyExc_KeyError, "");
        return NULL;
    }
    else {
        return PyUnicode_DecodeUTF8(v.str, v.len, NULL);
    }
}


static PyObject* clear(dictObj* self, PyObject* args){
    /*
    This function is called when dict.clear() is invoked. It takes an optional list argument which (if given)
    must contain keys of str type with length of at most key_str_len. Goals is to delete all the keys present
    in the given list. If the key type and length constraint is not met then either it will raise a TypeError
    the first time the constraint is violated or just skip that key depending on whether FLAG_CLEAR_RET_EXC flag
    is set. FLAG_CLEAR_RET_EXC is set by default.

    If no argument is provided then, then a empty hashtable is created. Note that the PyObject is the same.
    */

    kbox_t k; int ret_val;
    PyObject* list=NULL;

    if (!PyArg_ParseTuple(args, "|O", &list))
        return NULL;

    if (!list) {
        _destroy(self);
        _create(self, self->ht->key_str_len, self->ht->val_str_len);
        self->value_iterator->ht = self->ht;
        self->value_iterator->iter_idx = 0;
        self->value_iterator->iter_num = 0;

        self->item_iterator->ht = self->ht;
        self->item_iterator->iter_idx = 0;
        self->item_iterator->iter_num = 0;
        return Py_BuildValue("");
    }

    if (!PyList_CheckExact(list)) {
        PyErr_SetString(PyExc_TypeError, "The first optional argument must be a list");
        return NULL;
    }

    Py_ssize_t len = PyList_Size(list);
    Py_ssize_t str_len;

    for (Py_ssize_t i=0; i<len; ++i){
        PyObject* item = PyList_GetItem(list, i);
        const char* item_str = PyUnicode_AsUTF8AndSize(item, &str_len);
        if ((item_str != NULL) && (str_len <= self->ht->key_str_len)) {
            k.str = item_str;
            k.len = str_len;
            mdict_del_map(self->ht, k, NULL);
        } else if (_get_flag(self->flags, FLAG_CLEAR_RET_EXC)) {
            char msg[50];
            sprintf(msg, "key string must have length at most %d", self->ht->key_str_len);
            PyErr_SetString(PyExc_TypeError, msg);
            return NULL;
        }
    }

    PyErr_Clear();
    return Py_BuildValue("");
}



static PyObject* get_keys(dictObj* self) {
    /*
    This is called when dict.get_keys() is called. It returns a list containing all keys.
    In case it fails to add a key into the list, a None object is instead added.
    */

    i_t len = self->ht->size;
    h_t* h = self->ht;

    PyObject* list = PyList_New(len);    
    if (!list) {
        PyErr_SetString(PyExc_MemoryError, "Insufficient memory : Failed to allocate the list object");
        return NULL;
    }

    int k_step_inc = h->k_step_increment;
    Py_ssize_t idx = 0;
    for (i_t i=0; idx<len; ++i) {
        if (!_flags_isempty(h->flags, i)) {
            kbox_t key = _get_key(h, GET_PTR(i, k_step_inc));
            PyObject* item = PyUnicode_DecodeUTF8(key.str, key.len, NULL);
            if (item != NULL) {
                PyList_SET_ITEM(list, idx, item);
            } else {
                PyList_SET_ITEM(list, idx, Py_BuildValue(""));
            }
            idx += 1;
        }
    }

    return list;
}

static PyObject* get_values(dictObj* self) {
    /*
    This is called when dict.get_values() is called. It returns a list containing all values.
    In case it fails to add a value into the list, a None object is instead added.
    */

    i_t len = self->ht->size;
    h_t* h = self->ht;

    PyObject* list = PyList_New(len);    
    if (!list) {
        PyErr_SetString(PyExc_MemoryError, "Insufficient memory : Failed to allocate the list object");
        return NULL;
    }

    int v_step_inc = h->v_step_increment;
    Py_ssize_t idx = 0;
    for (i_t i=0; idx<len; ++i) {
        if (!_flags_isempty(h->flags, i)) {
            vbox_t val = _get_val(h, GET_PTR(i, v_step_inc));
            PyObject* item = PyUnicode_DecodeUTF8(val.str, val.len, NULL);
            if (item != NULL) {
                PyList_SET_ITEM(list, idx, item);
            } else {
                PyList_SET_ITEM(list, idx, Py_BuildValue(""));
            }
            idx += 1;
        }
    }

    return list;
}

static PyObject* get_items(dictObj* self) {
    /*
    This is called when dict.get_items() is called. It returns a list containing all items (key,value).
    In case it fails to add an item into the list, a None object is instead added.
    */

    i_t len = self->ht->size;
    h_t* h = self->ht;

    PyObject* list = PyList_New(len);    
    if (!list) {
        PyErr_SetString(PyExc_MemoryError, "Insufficient memory : Failed to allocate the list object");
        return NULL;
    }

    int k_step_inc = h->k_step_increment;
    int v_step_inc = h->v_step_increment;
    Py_ssize_t idx = 0;
    for (i_t i=0; idx<len; ++i) {
        if (!_flags_isempty(h->flags, i)) {
            kbox_t key = _get_key(h, GET_PTR(i, k_step_inc));
            vbox_t val = _get_val(h, GET_PTR(i, v_step_inc));
            PyObject* item_key = PyUnicode_DecodeUTF8(key.str, key.len, NULL);
            PyObject* item_val = PyUnicode_DecodeUTF8(val.str, val.len, NULL);
            if (item_key != NULL && item_val != NULL) {
                PyObject* item_obj = PyTuple_Pack(2, item_key, item_val);
                if (item_obj != NULL) {
                    PyList_SET_ITEM(list, idx, item_obj);
                } else {
                    PyErr_SetString(PyExc_MemoryError, "Insufficient memory : Failed to add all (Key, value) pairs to the list");
                    Py_DECREF(list);
                    return NULL;                
                }
            } else {
                Py_XDECREF(item_key);
                Py_XDECREF(item_val);
                PyList_SET_ITEM(list, idx, Py_BuildValue(""));                
            }
            idx += 1;
        }
    }

    return list;
}

int _update_from_Pydict(dictObj* self, PyObject* dict) {
    /*
    This function updates the hashtable with items from a given Python Dictionary. In case the python
    dictionary contains an item with non-matching types, then either that item will be skipped or TypeError
    will be raised depending on whether FLAG_UPDATE_ARG_EXC is set or not. FLAG_UPDATE_ARG_EXC is set by default.
    */

    PyObject *key_obj, *value_obj;
    Py_ssize_t pos = 0;
    Py_ssize_t len;
    kbox_t key;
    vbox_t val;

    while (PyDict_Next(dict, &pos, &key_obj, &value_obj)) {
        key.str = PyUnicode_AsUTF8AndSize(key_obj, &len);
        if (key.str == NULL || len > self->ht->key_str_len) {
            if (_get_flag(self->flags, FLAG_UPDATE_ARG_EXC)) {
                char msg[80];
                sprintf(msg, "Python Dictionary must contain Key string objects of length at most %d", self->ht->key_str_len);
                PyErr_SetString(PyExc_TypeError, msg);
                return -1;
            } else
                continue;
        }
        else {
            key.len = len;
        }
        
        val.str = PyUnicode_AsUTF8AndSize(value_obj, &len);
        if (val.str == NULL || len > self->ht->val_str_len) {
            if (_get_flag(self->flags, FLAG_UPDATE_ARG_EXC)) {
                char msg[80];
                sprintf(msg, "Python Dictionary must contain Value string objects of length at most %d", self->ht->val_str_len);
                PyErr_SetString(PyExc_TypeError, msg);
                return -1;
            } else
                continue;
        }
        else {
            val.len = len;
        }

        mdict_set(self->ht, key, val);     
    }

    return 0;    
}

void _update_from_mdict(dictObj* self, dictObj* dict) {
    /*
    This function updates the hashtable with all the items from another dictionary (dict) of the same key, value type.
    */

    h_t* h = self->ht;
    h_t* h2 = dict->ht;
    Py_ssize_t idx = 0;
    int k_step_inc = h->k_step_increment;
    int v_step_inc = h->v_step_increment;

    for (i_t i=0; idx<h2->size; ++i) {
        if (!_flags_isempty(h2->flags, i)) {
            kbox_t key = _get_key(h2, GET_PTR(i, k_step_inc));
            vbox_t val = _get_val(h2, GET_PTR(i, v_step_inc));
            mdict_set(h, key, val);     
            idx += 1;
        }
    }
}


static PyObject* to_Pydict(dictObj* self) {
    /*
    Returns a newly created python dictionary after updating it with all the items from the hashtable.
    Raises MemoryError if the dictionary could not successfully populated.
    */

    h_t* h = self->ht;
    PyObject* dict = PyDict_New();
    int k_step_inc = h->k_step_increment;
    int v_step_inc = h->v_step_increment;

    if (dict != NULL) {
        Py_ssize_t idx = 0;
        for (i_t i=0; idx<h->size; ++i) {
            if (!_flags_isempty(h->flags, i)) {
                kbox_t key = _get_key(h, GET_PTR(i, k_step_inc));
                vbox_t val = _get_val(h, GET_PTR(i, v_step_inc));
                PyObject* item_key = PyUnicode_DecodeUTF8(key.str, key.len, NULL);
                PyObject* item_val = PyUnicode_DecodeUTF8(val.str, val.len, NULL);
                if (item_key != NULL && item_val != NULL) {
                    if (PyDict_SetItem(dict, item_key, item_val) == -1) {
                        if (_get_flag(self->flags, FLAG_PYDICT_ARG_EXC)) {    
                            PyErr_SetString(PyExc_MemoryError, "Insufficient memory : Could not add all (key, value) pairs to the Python Dictionary object");
                            Py_DECREF(dict);
                            return NULL;
                        } else
                            return dict;
                    }
                } else if (_get_flag(self->flags, FLAG_PYDICT_ARG_EXC)) {
                    PyErr_SetString(PyExc_UnicodeDecodeError, "Could not decode UTF8 using PyUnicode_DecodeUTF8 function"); 
                    Py_DECREF(dict);
                    Py_XDECREF(item_key);
                    Py_XDECREF(item_val);
                    return NULL;                   
                } else {
                    Py_XDECREF(item_key);
                    Py_XDECREF(item_val);
                }
                idx += 1;
            }
        }
        return dict;
    } else {
        PyErr_SetString(PyExc_MemoryError, "Insufficient memory : Could not allocate the Python Dictionary object");
        return NULL;
    }
}


static int _contains_(dictObj* self, PyObject* key) {
    /*
    This function is called for the python expression 'k in dict' . k must be of the same type as the hashtable keys.
    */

    kbox_t k; i_t idx; Py_ssize_t len;
    k.str = PyUnicode_AsUTF8AndSize(key, &len);

    if (k.str == NULL || len > self->ht->key_str_len) {
        char msg[50];
        sprintf(msg, "Key needs to be a string of size at most %d", self->ht->key_str_len);
        PyErr_SetString(PyExc_TypeError, msg);
        return -1;
    }

    k.len = len;
    mdict_get_map(self->ht, k, &idx);
    if (idx != self->ht->num_buckets)
        return 1;
    else
        return 0;
}

static int _len_(dictObj* self) {
    /*
    This function is called when len(dict) is called. It returns the total number of items present.
    */
    return self->ht->size;
}


static PyObject* mapping_get(dictObj* self, PyObject* key){
    /*
    This function is invoked when dict[k] is called to return the corresponding value if present. If not present, a KeyError
    is raised if flag FLAG_GET_RET_EXC is set and otherwise, a None is returned. By default FLAG_GET_RET_EXC 
    is not set.
    */
    
    kbox_t k; vbox_t v; i_t idx; Py_ssize_t len;

    k.str = PyUnicode_AsUTF8AndSize(key, &len);

    if (k.str == NULL || len > self->ht->key_str_len) {
        char msg[50];
        sprintf(msg, "Key needs to be a string of size at most %d", self->ht->key_str_len);
        PyErr_SetString(PyExc_TypeError, msg);
        return NULL;
    }

    k.len = len;
    if (self->temp_isvalid && (k.len == self->temp_key.len) && _strncmp(k.str, self->temp_key.str, k.len)) {
        return PyUnicode_DecodeUTF8(self->temp_val.str, self->temp_val.len, NULL);
    } else {
        v = mdict_get_map(self->ht, k, &idx);
        if (idx != self->ht->num_buckets)
            return PyUnicode_DecodeUTF8(v.str, v.len, NULL);
        else {
            if (!_get_flag(self->flags, FLAG_GET_RET_EXC))
                return Py_BuildValue("");

            PyErr_SetString(PyExc_KeyError, "");
            return NULL;
        }
    }
}

static int mapping_set(dictObj* self, PyObject* key, PyObject* val){
    /*
    This is invoked for the python expression d[key] = value. Both key and value must be of the hashtable type.
    */

    vbox_t v; kbox_t k; Py_ssize_t len;

    k.str = PyUnicode_AsUTF8AndSize(key, &len);
    if (k.str == NULL || len > self->ht->key_str_len) {
        char msg[55];
        sprintf(msg, "Key needs to be a string of size at most %d", self->ht->key_str_len);
        PyErr_SetString(PyExc_TypeError, msg);
        return -1;
    }
    k.len = len;

    v.str = PyUnicode_AsUTF8AndSize(val, &len);
    if (v.str == NULL || len > self->ht->val_str_len) {
        char msg[55];
        sprintf(msg, "Value needs to be a string of size at most %d", self->ht->val_str_len);
        PyErr_SetString(PyExc_TypeError, msg);
        return -1;
    }
    v.len = len;

    mdict_set(self->ht, k, v);     

    if (self->temp_isvalid && (k.len == self->temp_key.len) && _strncmp(k.str, self->temp_key.str, k.len)) { // This logic supports that setting a value does not necessarily cache (key, val) pair and that the cache is mainly for the iterator.
        self->temp_val = v;
    }

    return 0;
}

static PyObject* mdict_iter(dictObj* self) {
    /*
    Returns an iterator for keys when __iter__(dict) is called 
    */

    Py_INCREF(self);
    self->iter_idx = 0;
    self->iter_num = 0;
    return (PyObject *) self;    
}


static PyObject* mdict_iternext(dictObj* self) {
    /*
    Iterates over the keys. When __next__(dict) is called, this function returns the next key. 
    */

    if (self->iter_num >= self->ht->size) {
        PyErr_SetNone(PyExc_StopIteration);
        self->temp_isvalid = false;
        return NULL;
    }

    h_t* h = self->ht;
    int k_step_inc = h->k_step_increment;
    int v_step_inc = h->v_step_increment;

    for (i_t i=self->iter_idx; ; ++i) {
        if (!_flags_isempty(h->flags, i)) {
            self->temp_key = _get_key(h, GET_PTR(i, k_step_inc));
            self->temp_val = _get_val(h, GET_PTR(i, v_step_inc));
            self->iter_idx = i+1;
            self->iter_num += 1;
            self->temp_isvalid = true;
            break;
        }
    }

    return PyUnicode_DecodeUTF8(self->temp_key.str, self->temp_key.len, NULL);
}


static PyObject* update(dictObj* self, PyObject* args);

static PyObject* get_value_iterator(dictObj* self) {
    /*
    Returns the value iterator
    */

    Py_INCREF((PyObject*) self->value_iterator);
    return (PyObject*) self->value_iterator;
}

static PyObject* get_item_iterator(dictObj* self) {
    /*
    Returns the item iterator
    */

    Py_INCREF((PyObject*) self->item_iterator);
    return (PyObject*) self->item_iterator;    
}

static PyObject* copy(dictObj* self) {
    /*
    Returns a new microdictionary object containing all items present in this hashtable when dict.copy() is called.
    */

    PyObject* args = Py_BuildValue("ii", self->ht->key_str_len, self->ht->val_str_len);
    dictObj* new_obj = (dictObj *) PyObject_CallObject(((PyObject *) self)->ob_type, args);
    Py_DECREF(args);
    _update_from_mdict(new_obj, self);
    return (PyObject*) new_obj;
}

static PyObject* map(dictObj* self, PyObject* args) { 
    /*
    Experimental status.
    */

    PyObject* list = NULL;
    PyObject* func = NULL;
    PyObject* output_list;

    if (!PyArg_ParseTuple(args, "O|O", &list, &func))
        return NULL;

    if (!PyList_CheckExact(list)) {
        return NULL;
    }

    Py_ssize_t size = PyList_Size(list);
    int str_len;
    output_list = PyList_New(size);
    kbox_t key;

    for (int i=0; i<size; ++i){
        key.str = PyUnicode_AsUTF8AndSize(PyList_GET_ITEM(list, i), &str_len);
        if (key.str == NULL || (str_len > self->ht->key_str_len)) {
            PyList_SET_ITEM(output_list, i, Py_BuildValue(""));
            continue;
        }
        key.len = str_len;
        int ret_idx;
        vbox_t val = mdict_get_map(self->ht, key, &ret_idx);
        if (ret_idx != self->ht->num_buckets) {
            if (func == NULL)
                PyList_SET_ITEM(output_list, i, PyUnicode_DecodeUTF8(val.str, val.len, NULL));
            else {
                PyObject* result = PyObject_CallFunction(func, "s#", val.str, val.len);
                if (result != NULL)
                    PyList_SET_ITEM(output_list, i, result);                                        
                else
                    PyList_SET_ITEM(output_list, i, Py_BuildValue(""));
            }
        }
        else
            PyList_SET_ITEM(output_list, i, Py_BuildValue(""));
    }

    PyErr_Clear();
    return output_list;
}

static PyObject* item_len(dictObj* self) {
    /*
    Returns max length of key and value that the dictionary supports.
    */

    return PyTuple_Pack(2, PyLong_FromLong(self->ht->key_str_len), PyLong_FromLong(self->ht->val_str_len));
}


static PyMethodDef methods_str_str[] = {
    {"pop", del, METH_VARARGS, "deletes a key-value pair and pops its value"},
    {"clear", clear, METH_VARARGS, "clears the hashtable"},
    {"get_keys", get_keys, METH_VARARGS, "returns a list of all keys"},
    {"get_values", get_values, METH_VARARGS, "returns a list of all values"},
    {"get_items", get_items, METH_VARARGS, "returns a list of all key-value pairs"},
    {"to_Pydict", to_Pydict, METH_VARARGS, "returns a python dictionary created from the microdict"},
    {"update", update, METH_VARARGS, "Updates the microdict with all key-value pairs within the given input: Either a Python dictionary or another microdict"},
    {"values", get_value_iterator, METH_VARARGS, "Returns an iterator for iterating over values"},
    {"items", get_item_iterator, METH_VARARGS, "Returns an iterator for iterating over items"},
    {"copy", copy, METH_VARARGS, "Returns a deep copy of the hashtable"},
    {"item_len", item_len, METH_VARARGS, "Returns the tuple (KEY_MAX_LENGTH, VALUE_MAX_LENGTH"},
    // {"map", map, METH_VARARGS, "Updates the microdict with all key-value pairs within the given input: Either a Python dictionary or another microdict"},
    {NULL, NULL, 0, NULL}
};


static PySequenceMethods sequence_str_str = {
    _len_,                            /* sq_length */
    0,                                  /* sq_concat */
    0,                                  /* sq_repeat */
    0,                                  /* sq_item */
    0,                                  /* sq_slice */
    0,                                  /* sq_ass_item */
    0,                                  /* sq_ass_slice */
    (objobjproc) _contains_,           /* sq_contains */
};

static PyMappingMethods mapping_str_str = {
    0, /*mp_length*/
    (binaryfunc)mapping_get, /*mp_subscript*/
    (objobjargproc)mapping_set, /*mp_ass_subscript*/
};


static PyTypeObject dictType_str_str = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "str->str",
    .tp_doc = "str->str microdictionary", 
    .tp_basicsize = sizeof(dictObj),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_new = custom_new,
    .tp_init = (initproc) custom_init,
    .tp_dealloc = (destructor) custom_dealloc,
    .tp_methods = methods_str_str,
    .tp_as_sequence = &sequence_str_str,
    .tp_as_mapping = &mapping_str_str,
    .tp_iter = (getiterfunc) mdict_iter,
    .tp_iternext = (iternextfunc) mdict_iternext,
};



static PyObject* update(dictObj* self, PyObject* args) {
    /*
    Invoked when dict.update() is called. It takes an argument which must be either a Python dictionary or a microdictionary
    of the same type. It adds all the items from the argument dictionary given to its hashtable. See _update_from_Pydict and
    _update_from_mdict for further documentation.
    */

    PyObject* dict;
    bool is_pydict;

    if (!PyArg_ParseTuple(args, "O!", &PyDict_Type, &dict)) {
        is_pydict = false;
        if (!PyArg_ParseTuple(args, "O", &dict)) {
            return NULL;
        }

        if (PyObject_IsInstance(dict, (PyObject *) &dictType_str_str) != 1) {
            PyErr_SetString(PyExc_TypeError, "Argument needs to be either a (key,value) string microdictionary or (key, value) string Python dictionary");
            return NULL;
        }
 
    } else
        is_pydict = true;

    if (is_pydict) {
        if (_update_from_Pydict(self, dict) == -1)
            return NULL;
    } else {
        dictObj* _dict_ = (dictObj*) dict;
        if ((_dict_->ht->key_str_len <= self->ht->key_str_len) && (_dict_->ht->val_str_len <= self->ht->val_str_len)) {
            _update_from_mdict(self, _dict_);
        } else {
            char msg[210];
            sprintf(msg, "Incompatible microdictionary argument : Trying to update a microdictionary of key length = %d, value length = %d with another microdictionary of key length = %d, value length = %d", self->ht->key_str_len, self->ht->val_str_len, _dict_->ht->key_str_len, _dict_->ht->val_str_len);
            PyErr_SetString(PyExc_TypeError, msg);
            return NULL;
        }
    }

    PyErr_Clear();
    return Py_BuildValue("");
}


static struct PyModuleDef moduleDef_str_str =
{
    PyModuleDef_HEAD_INIT,
    "str_str microdictionary", /* name of module */
    NULL, // Documentation of the module
    -1,   /* size of per-interpreter state of the module, or -1 if the module keeps state in global variables. */
};

PyMODINIT_FUNC PyInit_str_str(void)
{
    PyObject *obj;

    if (PyType_Ready(&dictType_str_str) < 0)
        return NULL;

    if (PyType_Ready(&valueIterType_str_str) < 0)
        return NULL;

    if (PyType_Ready(&itemIterType_str_str) < 0)
        return NULL;

    obj = PyModule_Create(&moduleDef_str_str);
    if (obj == NULL)
        return NULL;

    Py_INCREF(&dictType_str_str);
    if (PyModule_AddObject(obj, "create", (PyObject *) &dictType_str_str) < 0) {
        Py_DECREF(&dictType_str_str);
        Py_DECREF(obj);
        return NULL;
    }

    return obj;
}