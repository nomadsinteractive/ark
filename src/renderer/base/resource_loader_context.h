#ifndef ARK_RENDERER_BASE_RESOURCE_LOADER_CONTEXT_H_
#define ARK_RENDERER_BASE_RESOURCE_LOADER_CONTEXT_H_

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/base/object_pool.h"
#include "core/base/generic_object_pool.h"
#include "core/inf/runnable.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "renderer/forwarding.h"

namespace ark {

class ARK_API ResourceLoaderContext {
private:
    template<typename T> class SynchronizedVariable : public Variable<T>, public Runnable {
    public:
        SynchronizedVariable(const sp<Variable<T>>& delegate)
            : _delegate(delegate), _value(delegate->val()), _consumed(false) {
        }

        virtual void run() override {
            if(_consumed)
                _value = _delegate->val();
            _consumed = false;
        }

        virtual T val() override {
            _consumed = true;
            return _value;
        }

    private:
        sp<Variable<T>> _delegate;
        T _value;
        bool _consumed;
    };

public:
    class Synchronizer {
    public:
        Synchronizer(const sp<RenderController>& delegate);

        template<typename T> sp<Variable<T>> synchronize(const sp<Variable<T>>& delegate) {
            const sp<SynchronizedVariable<T>> synchronized = sp<SynchronizedVariable<T>>::make(delegate);
            addPreUpdateRequest(synchronized);
            return synchronized;
        }

        void addPreUpdateRequest(const sp<Runnable>& task);

        void expire();

    private:
        sp<RenderController> _delegate;
        sp<Boolean::Impl> _expired;
    };

public:
    ResourceLoaderContext(const sp<Dictionary<document>>& documents, const sp<GLResourceManager>& glResourceManager, const sp<Executor>& executor, const sp<RenderController>& synchronizer);
    ~ResourceLoaderContext();

    const sp<Dictionary<document>>& documents() const;
    const sp<GLResourceManager>& glResourceManager() const;
    const sp<Executor>& executor() const;
    const sp<GLTextureLoader>& textureLoader() const;
    const sp<Synchronizer>& synchronizer() const;

    template<typename T> sp<ObjectPool<T>> getObjectPool() {
        return _generic_object_pool.getObjectPool<T>();
    }

private:
    sp<Dictionary<document>> _documents;
    sp<GLResourceManager> _gl_resource_manager;
    sp<Executor> _executor;
    sp<GLTextureLoader> _texture_loader;
    sp<Synchronizer> _synchronizer;

    GenericObjectPool _generic_object_pool;

};

}

#endif
