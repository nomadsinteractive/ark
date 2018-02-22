#include "renderer/base/resource_loader_context.h"

#include "core/inf/variable.h"
#include "core/base/memory_pool.h"

#include "renderer/base/gl_buffer.h"
#include "renderer/base/gl_texture_loader.h"
#include "renderer/base/render_controller.h"

namespace ark {

ResourceLoaderContext::ResourceLoaderContext(const sp<Dictionary<document>>& documents, const sp<GLResourceManager>& glResourceManager, const sp<Executor>& executor, const sp<RenderController>& renderController)
    : _documents(documents), _gl_resource_manager(glResourceManager), _executor(executor), _render_controller(renderController),
      _texture_loader(sp<GLTextureLoader>::make(glResourceManager)), _memory_pool(sp<MemoryPool>::make()), _object_pool(sp<ObjectPool>::make()),
      _context_expired(sp<Expired::Impl>::make(false))
{
}

ResourceLoaderContext::~ResourceLoaderContext()
{
    LOGD("");
    _context_expired->set(true);
}

const sp<Dictionary<document>>& ResourceLoaderContext::documents() const
{
    return _documents;
}

const sp<GLResourceManager>& ResourceLoaderContext::glResourceManager() const
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

}
