#include "python/impl/adapter/collision_callback_python_adapter.h"

#include "core/util/holder_util.h"

#include "app/base/collision_manifold.h"

#include "python/api.h"
#include "python/extension/py_cast.h"

namespace ark::plugin::python {

CollisionCallbackPythonAdapter::CollisionCallbackPythonAdapter(const PyInstance& callback)
    : _on_begin_contact(callback.hasAttr("on_begin_contact") ? callback.getAttr("on_begin_contact")
                                                             : callback.isCallable() ? PyInstance::own(callback.pyObject()) : PyInstance()),
      _on_end_contact(callback.hasAttr("on_end_contact") ? callback.getAttr("on_end_contact")
                                                           : PyInstance()),
      _collision_manifold(sp<CollisionManifold>::make(V3(), V3()))
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
        PyTuple_SetItem(args, 1, PythonInterpreter::instance()->toPyObject(_collision_manifold));

/* Check again, in case of GC */
        if(_on_begin_contact)
        {
            PyObject* ret = _on_begin_contact.call(args);
            if(ret)
                Py_DECREF(ret);
            else
                PythonInterpreter::instance()->logErr();
        }

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

/* Check again, in case of GC */
        if(_on_end_contact)
        {
            PyObject* ret = _on_end_contact.call(args);
            if(ret)
                Py_DECREF(ret);
            else
                PythonInterpreter::instance()->logErr();
        }

        Py_DECREF(args);
    }
}

void CollisionCallbackPythonAdapter::traverse(const Holder::Visitor& visitor)
{
    HolderUtil::visit(_on_begin_contact.ref(), visitor);
    HolderUtil::visit(_on_end_contact.ref(), visitor);
}

PyObject* CollisionCallbackPythonAdapter::toPyObject(const sp<RigidBody>& rigidBody) const
{
    const Class* objClass = rigidBody.ensureClass();
    if(PythonInterpreter::instance()->isPyObject(objClass->id()))
        return PythonInterpreter::instance()->toPyObject(objClass->cast(Box(rigidBody), objClass->id()));
    return PyCast::toPyObject(rigidBody);
}

}
