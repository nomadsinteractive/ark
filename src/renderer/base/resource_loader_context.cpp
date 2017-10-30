#include "renderer/base/resource_loader_context.h"

#include "core/inf/variable.h"
#include "core/impl/boolean/boolean_by_weak_ref.h"
#include "core/impl/boolean/boolean_or.h"

#include "renderer/base/gl_buffer.h"
#include "renderer/base/gl_texture_loader.h"
#include "renderer/base/render_controller.h"

namespace ark {

ResourceLoaderContext::ResourceLoaderContext(const sp<Dictionary<document>>& documents, const sp<GLResourceManager>& glResourceManager, const sp<Executor>& executor, const sp<RenderController>& synchronizer)
    : _documents(documents), _gl_resource_manager(glResourceManager), _executor(executor), _texture_loader(sp<GLTextureLoader>::make(glResourceManager)), _synchronizer(sp<Synchronizer>::make(synchronizer))
{
}

ResourceLoaderContext::~ResourceLoaderContext()
{
    LOGD("");
    _synchronizer->expire();
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

const sp<GLTextureLoader>& ResourceLoaderContext::textureLoader() const
{
    return _texture_loader;
}

const sp<ResourceLoaderContext::Synchronizer>& ResourceLoaderContext::synchronizer() const
{
    return _synchronizer;
}

ResourceLoaderContext::Synchronizer::Synchronizer(const sp<RenderController>& delegate)
    : _delegate(delegate), _expired(sp<Boolean::Impl>::make(false))
{
}

void ResourceLoaderContext::Synchronizer::addPreUpdateRequest(const sp<Runnable>& task)
{
    const sp<Boolean> expired = sp<BooleanOr>::make(_expired, sp<BooleanByWeakRef<Runnable>>::make(task, 1));
    _delegate->addPreUpdateRequest(task, expired);
}

void ResourceLoaderContext::Synchronizer::expire()
{
    _expired->set(true);
}

}
