#include "python/impl/adapter/collision_callback_python_adapter.h"

#include "app/base/collision_manifold.h"

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
      _args1(PyInstance::steal(PyTuple_New(1))), _args2(PyInstance::steal(PyTuple_New(2))),
      _collision_manifold(sp<CollisionManifold>::make(V()))
{
}

void CollisionCallbackPythonAdapter::onBeginContact(const sp<RigidBody>& rigidBody, const CollisionManifold& manifold)
{
    DCHECK_THREAD_FLAG();
    if(_on_begin_contact)
    {
        *_collision_manifold = manifold;
        PyTuple_SetItem(_args2, 0, PythonInterpreter::instance()->fromSharedPtr<RigidBody>(rigidBody));
        PyTuple_SetItem(_args2, 1, PythonInterpreter::instance()->fromSharedPtr<CollisionManifold>(_collision_manifold));

        PyObject* ret = _on_begin_contact.call(_args2);
        if(!ret)
            PythonInterpreter::instance()->logErr();

        PyTuple_SetItem(_args2, 0, Py_None);
        PyTuple_SetItem(_args2, 1, Py_None);

        Py_XDECREF(ret);
    }
}

void CollisionCallbackPythonAdapter::onEndContact(const sp<RigidBody>& rigidBody)
{
    DCHECK_THREAD_FLAG();
    if(_on_end_contact)
    {
        PyTuple_SetItem(_args1, 0, PythonInterpreter::instance()->fromSharedPtr<RigidBody>(rigidBody));

        PyObject* ret = _on_end_contact.call(_args1);
        if(!ret)
            PythonInterpreter::instance()->logErr();

        PyTuple_SetItem(_args1, 0, Py_None);

        Py_XDECREF(ret);
    }
}

}
}
}
