#include "python/impl/adapter/collision_callback_python_adapter.h"

#include "python/extension/python_interpreter.h"

#include "python/api.h"

namespace ark {
namespace plugin {
namespace python {

CollisionCallbackPythonAdapter::CollisionCallbackPythonAdapter(const PyInstance& callback)
    : _on_begin_contact(callback.hasAttr("on_begin_contact") ? PyInstance::steal(callback.getAttr("on_begin_contact"))
                                                             : PyInstance::borrow(Py_None)),
      _on_end_contact(callback.hasAttr("on_end_contact") ? PyInstance::steal(callback.getAttr("on_end_contact"))
                                                           : PyInstance::borrow(Py_None)),
      _args(PyInstance::steal(PyTuple_New(1)))
{
}

void CollisionCallbackPythonAdapter::onBeginContact(const sp<RigidBody>& rigidBody)
{
    DCHECK_THREAD_FLAG();
    if(_on_begin_contact)
    {
        PyTuple_SetItem(_args, 0, PythonInterpreter::instance()->fromSharedPtr<RigidBody>(rigidBody));

        PyObject* ret = _on_begin_contact.call(_args);
        if(!ret)
            PythonInterpreter::instance()->logErr();

        PyTuple_SetItem(_args, 0, Py_None);

        Py_XDECREF(ret);
    }
}

void CollisionCallbackPythonAdapter::onEndContact(const sp<RigidBody>& rigidBody)
{
    DCHECK_THREAD_FLAG();
    if(_on_end_contact)
    {
        PyTuple_SetItem(_args, 0, PythonInterpreter::instance()->fromSharedPtr<RigidBody>(rigidBody));

        PyObject* ret = _on_end_contact.call(_args);
        if(!ret)
            PythonInterpreter::instance()->logErr();

        PyTuple_SetItem(_args, 0, Py_None);

        Py_XDECREF(ret);
    }
}

}
}
}
