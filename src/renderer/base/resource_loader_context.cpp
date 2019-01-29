#include "renderer/base/resource_loader_context.h"

#include "core/base/memory_pool.h"
#include "core/inf/variable.h"
#include "core/util/log.h"

#include "renderer/base/buffer.h"
#include "renderer/base/texture_bundle.h"
#include "renderer/base/render_controller.h"

namespace ark {

ResourceLoaderContext::ResourceLoaderContext(const sp<Dictionary<document>>& documents, const sp<BitmapBundle>& imageResource,
                                             const sp<Executor>& executor, const sp<RenderController>& renderController)
    : _documents(documents), _images(imageResource), _executor(executor), _render_controller(renderController),
      _texture_bundle(sp<TextureBundle>::make(renderController)), _memory_pool(sp<MemoryPool>::make()), _object_pool(sp<ObjectPool>::make()),
      _disposed(sp<Boolean::Impl>::make(false))
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

const sp<BitmapBundle>& ResourceLoaderContext::images() const
{
    return _images;
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

const sp<MemoryPool>& ResourceLoaderContext::memoryPool() const
{
    return _memory_pool;
}

const sp<ObjectPool>& ResourceLoaderContext::objectPool() const
{
    return _object_pool;
}

sp<Boolean> ResourceLoaderContext::disposed() const
{
    return _disposed;
}

}
