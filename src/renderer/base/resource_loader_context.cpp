#include "renderer/base/resource_loader_context.h"

#include "core/base/memory_pool.h"
#include "core/inf/variable.h"
#include "core/util/log.h"

#include "renderer/base/gl_buffer.h"
#include "renderer/base/gl_texture_loader.h"
#include "renderer/base/render_controller.h"

namespace ark {

ResourceLoaderContext::ResourceLoaderContext(const sp<Dictionary<document>>& documents, const sp<ImageResource>& imageResource, const sp<GLResourceManager>& glResourceManager, const sp<Executor>& executor, const sp<RenderController>& renderController)
    : _documents(documents), _images(imageResource), _gl_resource_manager(glResourceManager), _executor(executor), _render_controller(renderController),
      _texture_loader(sp<GLTextureLoader>::make(glResourceManager)), _memory_pool(sp<MemoryPool>::make()), _object_pool(sp<ObjectPool>::make()),
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

const sp<ImageResource>& ResourceLoaderContext::images() const
{
    return _images;
}

const sp<GLResourceManager>& ResourceLoaderContext::resourceManager() const
{
    return _gl_resource_manager;
}

const sp<Executor>& ResourceLoaderContext::executor() const
{
    return _executor;
}

const sp<RenderController>& ResourceLoaderContext::renderController() const
{
    return _render_controller;
}

const sp<GLTextureLoader>& ResourceLoaderContext::textureLoader() const
{
    return _texture_loader;
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
