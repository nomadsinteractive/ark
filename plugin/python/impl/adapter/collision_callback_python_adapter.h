#ifndef ARK_PLUGIN_PYTHON_IMPL_ADAPTER_COLLISION_CALLBACK_PYTHON_ADAPTER_H_
#define ARK_PLUGIN_PYTHON_IMPL_ADAPTER_COLLISION_CALLBACK_PYTHON_ADAPTER_H_

#include "core/types/shared_ptr.h"

#include "app/inf/collision_callback.h"

#include "python/extension/py_instance.h"
#include "python/extension/py_garbage_collector.h"

namespace ark {
namespace plugin {
namespace python {

class CollisionCallbackPythonAdapter : public CollisionCallback, public PyGarbageCollector, Implements<CollisionCallbackPythonAdapter, CollisionCallback, PyGarbageCollector> {
public:
    CollisionCallbackPythonAdapter(const PyInstance& callback);

    virtual void onBeginContact(const sp<RigidBody>& rigidBody, const CollisionManifold& manifold) override;
    virtual void onEndContact(const sp<RigidBody>& rigidBody) override;

    virtual int traverse(visitproc visit, void* arg) override;
    virtual int clear() override;

private:
    sp<PyInstance> _on_begin_contact;
    sp<PyInstance> _on_end_contact;

    sp<CollisionManifold> _collision_manifold;

};

}
}
}

#endif
