#include "python/impl/adapter/collision_callback_python_adapter.h"

#include "core/util/holder_util.h"

#include "app/base/collision_manifold.h"
#include "app/base/rigid_body.h"

#include "python/api.h"
#include "python/extension/py_cast.h"

namespace ark::plugin::python {

CollisionCallbackPythonAdapter::CollisionCallbackPythonAdapter(const PyInstance& callback)
    : _on_begin_contact(callback.hasAttr("on_begin_contact") ? callback.getAttr("on_begin_contact") : PyInstance()),
      _on_end_contact(callback.hasAttr("on_end_contact") ? callback.getAttr("on_end_contact") : PyInstance())
{
}

void CollisionCallbackPythonAdapter::onBeginContact(const RigidBody& rigidBody, const CollisionManifold& manifold)
{
    DCHECK_THREAD_FLAG();
    if(_on_begin_contact)
    {
        PyObject* args = PyTuple_New(2);

        PyTuple_SetItem(args, 0, toPyObject(rigidBody.makeShadow()));
        PyTuple_SetItem(args, 1, PyCast::toPyObject(sp<CollisionManifold>::make(manifold)));

/* Check again, in case of GC */
        if(_on_begin_contact)
        {
            if(PyObject* ret = _on_begin_contact.call(args))
                Py_DECREF(ret);
            else
                PythonInterpreter::instance().logErr();
        }

        Py_DECREF(args);
    }
}

void CollisionCallbackPythonAdapter::onEndContact(const RigidBody& rigidBody)
{
    DCHECK_THREAD_FLAG();
    if(_on_end_contact)
    {
        PyObject* args = PyTuple_New(1);
        PyTuple_SetItem(args, 0, toPyObject(rigidBody.makeShadow()));

/* Check again, in case of GC */
        if(_on_end_contact)
        {
            if(PyObject* ret = _on_end_contact.call(args))
                Py_DECREF(ret);
            else
                PythonInterpreter::instance().logErr();
        }

        Py_DECREF(args);
    }
}

PyObject* CollisionCallbackPythonAdapter::toPyObject(const sp<RigidBody>& rigidBody) const
{
    const Class* objClass = rigidBody.getClass();
    if(PythonInterpreter::instance().isPyObject(objClass->id()))
        return PythonInterpreter::instance().toPyObject(objClass->cast(Box(rigidBody), objClass->id()));
    return PyCast::toPyObject(rigidBody);
}

}
