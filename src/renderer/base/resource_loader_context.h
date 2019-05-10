#ifndef ARK_RENDERER_BASE_RESOURCE_LOADER_CONTEXT_H_
#define ARK_RENDERER_BASE_RESOURCE_LOADER_CONTEXT_H_

#include <unordered_map>

#include "core/forwarding.h"
#include "core/collection/list.h"
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
public:
    ResourceLoaderContext(const sp<Dictionary<document>>& documents, const sp<BitmapBundle>& images, const sp<Executor>& executor, const sp<RenderController>& renderController);
    ~ResourceLoaderContext();

    const sp<Dictionary<document>>& documents() const;
    const sp<BitmapBundle>& images() const;

    const sp<RenderController>& renderController() const;
    const sp<Executor>& executor() const;
    const sp<TextureBundle>& textureBundle() const;
    const sp<MemoryPool>& memoryPool() const;
    const sp<ObjectPool>& objectPool() const;
    sp<Boolean> disposed() const;

    template<typename T> sp<Variable<T>> synchronize(const sp<Variable<T>>& delegate) {
        return _render_controller->synchronize<T>(delegate);
    }

private:
    sp<Dictionary<document>> _documents;
    sp<BitmapBundle> _images;
    sp<Executor> _executor;
    sp<RenderController> _render_controller;
    sp<TextureBundle> _texture_bundle;
    sp<MemoryPool> _memory_pool;
    sp<ObjectPool> _object_pool;

    sp<Boolean::Impl> _disposed;

};

}

#endif
