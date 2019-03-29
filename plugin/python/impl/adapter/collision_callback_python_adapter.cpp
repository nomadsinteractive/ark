#include "python/impl/adapter/collision_callback_python_adapter.h"

#include "app/base/collision_manifold.h"

#include "python/api.h"
#include "python/extension/python_interpreter.h"

namespace ark {
namespace plugin {
namespace python {

CollisionCallbackPythonAdapter::CollisionCallbackPythonAdapter(const PyInstance& callback)
    : _on_begin_contact(callback.hasAttr("on_begin_contact") ? callback.getAttr("on_begin_contact")
                                                             : sp<PyInstance>::null()),
      _on_end_contact(callback.hasAttr("on_end_contact") ? callback.getAttr("on_end_contact")
                                                           : sp<PyInstance>::null()),
      _collision_manifold(sp<CollisionManifold>::make(V()))
{
}

void CollisionCallbackPythonAdapter::onBeginContact(const sp<RigidBody>& rigidBody, const CollisionManifold& manifold)
{
    DCHECK_THREAD_FLAG();
    if(_on_begin_contact)
    {
        *_collision_manifold = manifold;

        PyObject* args = PyTuple_New(2);

        PyTuple_SetItem(args, 0, toPyObject(rigidBody));
        PyTuple_SetItem(args, 1, PythonInterpreter::instance()->fromSharedPtr<CollisionManifold>(_collision_manifold));

        PyObject* ret = _on_begin_contact->call(args);
        if(ret)
            Py_DECREF(ret);
        else
            PythonInterpreter::instance()->logErr();

        Py_DECREF(args);
    }
}

void CollisionCallbackPythonAdapter::onEndContact(const sp<RigidBody>& rigidBody)
{
    DCHECK_THREAD_FLAG();
    if(_on_end_contact)
    {
        PyObject* args = PyTuple_New(1);
        PyTuple_SetItem(args, 0, toPyObject(rigidBody));

        PyObject* ret = _on_end_contact->call(args);
        if(ret)
            Py_DECREF(ret);
        else
            PythonInterpreter::instance()->logErr();

        Py_DECREF(args);
    }
}

int CollisionCallbackPythonAdapter::traverse(visitproc visit, void* arg)
{
    if(_on_begin_contact)
        Py_VISIT(_on_begin_contact->object());
    if(_on_end_contact)
        Py_VISIT(_on_end_contact->object());
    return 0;
}

int CollisionCallbackPythonAdapter::clear()
{
    if(_on_end_contact)
    {
        _on_begin_contact->deref();
        _on_begin_contact = nullptr;
    }
    if(_on_end_contact)
    {
        _on_end_contact->deref();
        _on_end_contact = nullptr;
    }
    return 0;
}

PyObject* CollisionCallbackPythonAdapter::toPyObject(const sp<RigidBody>& rigidBody) const
{
    TypeId concreteTypeId = rigidBody.interfaces()->typeId();
    if(PythonInterpreter::instance()->isPyObject(concreteTypeId))
        return PythonInterpreter::instance()->toPyObject(rigidBody.pack().toConcrete());
    return PythonInterpreter::instance()->fromSharedPtr<RigidBody>(rigidBody);
}

}
}
}
