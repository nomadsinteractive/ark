#pragma once

#include "app/inf/collision_callback.h"
#include "core/inf/debris.h"
#include "core/types/implements.h"

#include "python/extension/py_instance.h"

namespace ark::plugin::python {

class [[deprecated]] CollisionCallbackPython final : public CollisionCallback, public Debris, public Implements<CollisionCallbackPython, CollisionCallback, Debris> {
public:
    CollisionCallbackPython(const PyInstance& delegate);

    void onBeginContact(const Rigidbody& rigidBody, const CollisionManifold& manifold) override;
    void onEndContact(const Rigidbody& rigidBody) override;

    void traverse(const Visitor& visitor) override;

private:
    PyInstance _on_begin_contact;
    PyInstance _on_end_contact;
};

}
