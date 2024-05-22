#pragma once

#include "core/inf/holder.h"
#include "core/types/shared_ptr.h"

#include "app/inf/collision_callback.h"

#include "python/extension/py_instance.h"

namespace ark::plugin::python {

//[[script::bindings::holder]]
class CollisionCallbackPythonAdapter : public CollisionCallback, public Holder, Implements<CollisionCallbackPythonAdapter, CollisionCallback, Holder> {
public:
    CollisionCallbackPythonAdapter(const PyInstance& callback);

    virtual void onBeginContact(const sp<RigidBody>& rigidBody, const CollisionManifold& manifold) override;
    virtual void onEndContact(const sp<RigidBody>& rigidBody) override;

    virtual void traverse(const Visitor& visitor) override;

private:
    PyObject* toPyObject(const sp<RigidBody>& rigidBody) const;

private:
    PyInstance _on_begin_contact;
    PyInstance _on_end_contact;

    sp<CollisionManifold> _collision_manifold;

};

}
