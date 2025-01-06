#include "python/impl/adapter/collision_callback_python.h"

#include "app/base/collision_manifold.h"
#include "app/components/rigidbody.h"

#include "python/api.h"
#include "python/extension/py_cast.h"

namespace ark::plugin::python {

namespace {

PyObject* toPyObject(const sp<Rigidbody>& rigidBody)
{
    if(const Class* objClass = rigidBody.getClass(); PythonExtension::instance().isPyObject(objClass->id()))
        return PythonExtension::instance().toPyObject(objClass->cast(Box(rigidBody), objClass->id()));
    return PyCast::toPyObject(rigidBody);
}

}

CollisionCallbackPython::CollisionCallbackPython(const PyInstance& delegate)
    : _on_begin_contact(delegate.hasAttr("on_begin_contact") ? delegate.getAttr("on_begin_contact") : PyInstance()),
      _on_end_contact(delegate.hasAttr("on_end_contact") ? delegate.getAttr("on_end_contact") : PyInstance())
{
}

void CollisionCallbackPython::onBeginContact(const Rigidbody& rigidBody, const CollisionManifold& manifold)
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
                PythonExtension::instance().logErr();
        }

        Py_DECREF(args);
    }
}

void CollisionCallbackPython::onEndContact(const Rigidbody& rigidBody)
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
                PythonExtension::instance().logErr();
        }

        Py_DECREF(args);
    }
}

void CollisionCallbackPython::traverse(const Visitor& visitor)
{
    _on_begin_contact.traverse(visitor);
    _on_end_contact.traverse(visitor);
}

}
