#ifndef ARK_RENDERER_BASE_RESOURCE_LOADER_CONTEXT_H_
#define ARK_RENDERER_BASE_RESOURCE_LOADER_CONTEXT_H_

#include <unordered_map>

#include "core/forwarding.h"
#include "core/collection/list.h"
#include "core/base/api.h"
#include "core/inf/runnable.h"
#include "core/inf/variable.h"
#include "core/impl/boolean/boolean_by_weak_ref.h"
#include "core/impl/dictionary/loader_bundle.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "renderer/forwarding.h"
#include "renderer/base/render_controller.h"

namespace ark {

class ARK_API ResourceLoaderContext {
public:
    ResourceLoaderContext(sp<Dictionary<document>> documents, sp<BitmapLoaderBundle> bitmapBundle, sp<BitmapLoaderBundle> bitmapBoundsBundle, sp<ExecutorThreadPool> executor,
                          sp<RenderController> renderController);
    ~ResourceLoaderContext();

    const sp<Dictionary<document>>& documents() const;

    const sp<RenderController>& renderController() const;
    const sp<ExecutorThreadPool>& executorThreadPool() const;

    const sp<BitmapLoaderBundle>& bitmapBundle() const;
    const sp<BitmapLoaderBundle>& bitmapBoundsBundle() const;
    const sp<TextureBundle>& textureBundle() const;

    sp<Boolean> disposed() const;

    template<typename T> sp<Variable<T>> synchronize(sp<Variable<T>> delegate, sp<Boolean> disposed = nullptr) {
        return _render_controller->synchronize<T>(std::move(delegate), std::move(disposed));
    }

private:
    sp<Dictionary<document>> _documents;
    sp<BitmapLoaderBundle> _bitmap_bundle;
    sp<BitmapLoaderBundle> _bitmap_bounds_bundle;
    sp<ExecutorThreadPool> _executor_thread_pool;
    sp<RenderController> _render_controller;
    sp<TextureBundle> _texture_bundle;

    sp<Boolean::Impl> _discarded;

};

}

#endif
