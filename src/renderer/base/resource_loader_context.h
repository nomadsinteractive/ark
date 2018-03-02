#ifndef ARK_RENDERER_BASE_RESOURCE_LOADER_CONTEXT_H_
#define ARK_RENDERER_BASE_RESOURCE_LOADER_CONTEXT_H_

#include <map>

#include "core/forwarding.h"
#include "core/collection/expirable_item_list.h"
#include "core/base/api.h"
#include "core/base/object_pool.h"
#include "core/inf/runnable.h"
#include "core/inf/variable.h"
#include "core/impl/boolean/boolean_by_weak_ref.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "renderer/forwarding.h"
#include "renderer/base/render_controller.h"

namespace ark {

class ARK_API ResourceLoaderContext {
private:
    template<typename T> class Synchronizer : public Runnable, public Boolean {
    private:
        class SynchronizedVariable : public Variable<T> {
        public:
            SynchronizedVariable(const sp<Variable<T>>& delegate)
                : _delegate(delegate), _value(delegate->val()), _consumed(false) {
            }

            void update() {
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
        sp<Variable<T>> synchronize(const sp<Variable<T>>& delegate, const sp<Boolean>& expired) {
            const sp<SynchronizedVariable> synchronized = sp<SynchronizedVariable>::make(delegate);
            const sp<Boolean> s = expired ? expired : sp<Boolean>::adopt(new BooleanByWeakRef<SynchronizedVariable>(synchronized, 1));
            _variables.push_back(synchronized, s);
            return synchronized;
        }

        virtual bool val() override {
            return _variables.size() == 0;
        }

        virtual void run() override {
            for(SynchronizedVariable& i : _variables)
                i.update();
        }

    private:
        ExpirableItemList<SynchronizedVariable> _variables;
    };

public:
    ResourceLoaderContext(const sp<Dictionary<document>>& documents, const sp<GLResourceManager>& glResourceManager, const sp<Executor>& executor, const sp<RenderController>& renderController);
    ~ResourceLoaderContext();

    const sp<Dictionary<document>>& documents() const;
    const sp<GLResourceManager>& glResourceManager() const;
    const sp<Executor>& executor() const;
    const sp<RenderController>& renderController() const;
    const sp<GLTextureLoader>& textureLoader() const;
    const sp<MemoryPool>& memoryPool() const;
    const sp<ObjectPool>& objectPool() const;

    template<typename T> sp<Variable<T>> synchronize(const sp<Variable<T>>& delegate, const sp<Boolean>& expired = nullptr) {
        return getSynchronizer<T>()->synchronize(delegate, expired);
    }

private:
    template<typename T> sp<Synchronizer<T>> getSynchronizer() {
        const auto iter = _synchronizers.find(Type<T>::id());
        if(iter == _synchronizers.end()) {
            const sp<Synchronizer<T>> synchronizer = sp<Synchronizer<T>>::make();
            _synchronizers.emplace(Type<T>::id(), synchronizer);
            _render_controller->addPreUpdateRequest(synchronizer, _context_expired);
            return synchronizer;
        }
        return iter->second;
    }

private:
    sp<Dictionary<document>> _documents;
    sp<GLResourceManager> _gl_resource_manager;
    sp<Executor> _executor;
    sp<RenderController> _render_controller;
    sp<GLTextureLoader> _texture_loader;
    sp<MemoryPool> _memory_pool;
    sp<ObjectPool> _object_pool;

    sp<Expired::Impl> _context_expired;
    std::map<TypeId, sp<Runnable>> _synchronizers;

};

}

#endif
