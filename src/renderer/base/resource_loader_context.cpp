#include "renderer/base/resource_loader_context.h"

#include "core/base/memory_pool.h"
#include "core/inf/variable.h"
#include "core/util/log.h"

#include "renderer/base/buffer.h"
#include "renderer/base/texture_bundle.h"
#include "renderer/base/render_controller.h"

namespace ark {

ResourceLoaderContext::ResourceLoaderContext(const sp<Dictionary<document>>& documents, const sp<BitmapLoaderBundle>& bitmapBundle, const sp<BitmapLoaderBundle>& bitmapBoundsBundle,
                                             const sp<Executor>& executor, const sp<RenderController>& renderController)
    : _documents(documents), _bitmap_bundle(bitmapBundle), _bitmap_bounds_bundle(bitmapBoundsBundle), _executor(executor), _render_controller(renderController),
      _texture_bundle(sp<TextureBundle>::make(renderController)), _disposed(sp<Boolean::Impl>::make(false))
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

const sp<Executor>& ResourceLoaderContext::executor() const
{
    return _executor;
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
