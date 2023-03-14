#include "renderer/base/resource_loader_context.h"

#include "core/base/memory_pool.h"
#include "core/inf/variable.h"
#include "core/util/log.h"

#include "renderer/base/buffer.h"
#include "renderer/base/texture_bundle.h"
#include "renderer/base/render_controller.h"

namespace ark {

ResourceLoaderContext::ResourceLoaderContext(sp<Dictionary<document>> documents, sp<BitmapLoaderBundle> bitmapBundle, sp<BitmapLoaderBundle> bitmapBoundsBundle,
                                             sp<ExecutorThreadPool> executor, sp<RenderController> renderController)
    : _documents(std::move(documents)), _bitmap_bundle(std::move(bitmapBundle)), _bitmap_bounds_bundle(std::move(bitmapBoundsBundle)), _executor_thread_pool(std::move(executor)),
      _render_controller(std::move(renderController)), _texture_bundle(sp<TextureBundle>::make(_render_controller)), _disposed(sp<Boolean::Impl>::make(false))
{
}

ResourceLoaderContext::~ResourceLoaderContext()
{
    LOGD("");
    _disposed->set(true);
}

const sp<Dictionary<document>>& ResourceLoaderContext::documents() const
{
    return _documents;
}

const sp<BitmapLoaderBundle>& ResourceLoaderContext::bitmapBundle() const
{
    return _bitmap_bundle;
}

const sp<BitmapLoaderBundle>& ResourceLoaderContext::bitmapBoundsBundle() const
{
    return _bitmap_bounds_bundle;
}

const sp<ExecutorThreadPool>& ResourceLoaderContext::executorThreadPool() const
{
    return _executor_thread_pool;
}

const sp<RenderController>& ResourceLoaderContext::renderController() const
{
    return _render_controller;
}

const sp<TextureBundle>& ResourceLoaderContext::textureBundle() const
{
    return _texture_bundle;
}

sp<Boolean> ResourceLoaderContext::disposed() const
{
    return _disposed;
}

}
