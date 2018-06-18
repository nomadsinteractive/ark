#ifndef ARK_PLUGIN_PYTHON_IMPL_ADAPTER_COLLISION_CALLBACK_PYTHON_ADAPTER_H_
#define ARK_PLUGIN_PYTHON_IMPL_ADAPTER_COLLISION_CALLBACK_PYTHON_ADAPTER_H_

#include "core/types/shared_ptr.h"

#include "app/inf/collision_callback.h"

#include "python/extension/py_instance.h"

namespace ark {
namespace plugin {
namespace python {

class CollisionCallbackPythonAdapter : public CollisionCallback, Implements<CollisionCallbackPythonAdapter, CollisionCallback> {
public:
    CollisionCallbackPythonAdapter(const PyInstance& callback);

    virtual void onBeginContact(const sp<RigidBody>& rigidBody, const CollisionManifold& manifold) override;
    virtual void onEndContact(const sp<RigidBody>& rigidBody) override;

private:
    PyInstance _on_begin_contact;
    PyInstance _on_end_contact;
    PyInstance _args1;
    PyInstance _args2;

    sp<CollisionManifold> _collision_manifold;

};

}
}
}

#endif
