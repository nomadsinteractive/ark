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
    ResourceLoaderContext(const sp<Dictionary<document>>& documents, const sp<BitmapBundle>& bitmapBundle, const sp<BitmapBundle>& bitmapBoundsBundle,
                          const sp<Executor>& executor, const sp<RenderController>& renderController);
    ~ResourceLoaderContext();

    const sp<Dictionary<document>>& documents() const;

    const sp<RenderController>& renderController() const;
    const sp<Executor>& executor() const;

    const sp<BitmapBundle>& bitmapBundle() const;
    const sp<BitmapBundle>& bitmapBoundsBundle() const;
    const sp<TextureBundle>& textureBundle() const;

    sp<Boolean> disposed() const;

    template<typename T> sp<Variable<T>> synchronize(const sp<Variable<T>>& delegate, const sp<Boolean>& disposed = nullptr) {
        return _render_controller->synchronize<T>(delegate, disposed);
    }

private:
    sp<Dictionary<document>> _documents;
    sp<BitmapBundle> _bitmap_bundle;
    sp<BitmapBundle> _bitmap_bounds_bundle;
    sp<Executor> _executor;
    sp<RenderController> _render_controller;
    sp<TextureBundle> _texture_bundle;

    sp<Boolean::Impl> _disposed;

};

}

#endif
