#pragma once


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

    const sp<BitmapLoaderBundle>& bitmapBundle() const;
    const sp<BitmapLoaderBundle>& bitmapBoundsBundle() const;
    const sp<TextureBundle>& textureBundle() const;

    sp<Boolean> discarded() const;

private:
    sp<Dictionary<document>> _documents;
    sp<BitmapLoaderBundle> _bitmap_bundle;
    sp<BitmapLoaderBundle> _bitmap_bounds_bundle;
    sp<RenderController> _render_controller;
    sp<TextureBundle> _texture_bundle;

    sp<Discarded> _discarded;

};

}
