#include "python/impl/duck/py_list_duck_type.h"

#include "python/extension/python_interpreter.h"

namespace ark::plugin::python {

PyListDuckType::PyListDuckType(PyInstance instance)
    : _instance(std::move(instance))
{
}

void PyListDuckType::to(sp<std::vector<Box>>& inst)
{
    std::vector<Box> boxes;
    const Py_ssize_t list_len = PyList_Size(_instance.pyObject());
    const PythonInterpreter& pi = PythonInterpreter::instance();
    for(Py_ssize_t i = 0; i < list_len; ++i)
    {
        PyObject* item = PyList_GET_ITEM(_instance.pyObject(), i);
        if(pi.isPyArkTypeObject(Py_TYPE(item)))
            boxes.push_back(*reinterpret_cast<PyArkType::Instance*>(item)->box);
    }
    inst = sp<std::vector<Box>>::make(std::move(boxes));
}

}
