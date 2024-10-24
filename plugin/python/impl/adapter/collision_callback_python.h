#pragma once

#include "app/inf/collision_callback.h"
#include "core/inf/debris.h"

#include "python/extension/py_instance.h"

namespace ark::plugin::python {

class CollisionCallbackPython final : public CollisionCallback, public Debris, public Implements<CollisionCallbackPython, CollisionCallback, Debris> {
public:
    CollisionCallbackPython(const PyInstance& delegate);

    void onBeginContact(const RigidBody& rigidBody, const CollisionManifold& manifold) override;
    void onEndContact(const RigidBody& rigidBody) override;

    void traverse(const Visitor& visitor) override;

private:
    PyInstance _on_begin_contact;
    PyInstance _on_end_contact;
};

}
