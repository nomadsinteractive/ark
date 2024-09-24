#pragma once

#include "app/inf/collision_callback.h"

#include "python/extension/py_instance.h"

namespace ark::plugin::python {

class CollisionCallbackPythonAdapter : public CollisionCallback, public Implements<CollisionCallbackPythonAdapter, CollisionCallback> {
public:
    CollisionCallbackPythonAdapter(const PyInstance& callback);

    void onBeginContact(const RigidBody& rigidBody, const CollisionManifold& manifold) override;
    void onEndContact(const RigidBody& rigidBody) override;

private:
    PyObject* toPyObject(const sp<RigidBody>& rigidBody) const;

private:
    PyInstance _on_begin_contact;
    PyInstance _on_end_contact;
};

}
