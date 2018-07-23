#include "renderer/base/gl_resource_manager.h"

#include "core/inf/array.h"
#include "core/inf/variable.h"
#include "core/impl/array/fixed_array.h"
#include "core/impl/array/dynamic_array.h"
#include "core/types/weak_ptr.h"
#include "core/util/log.h"

#include "renderer/base/gl_buffer.h"
#include "renderer/base/gl_context.h"
#include "renderer/base/gl_recycler.h"
#include "renderer/base/gl_shader.h"
#include "renderer/base/gl_snippet_delegate.h"
#include "renderer/base/gl_texture.h"
#include "renderer/base/graphics_context.h"
#include "renderer/impl/resource/gl_texture_resource.h"
#include "renderer/inf/gl_snippet.h"
#include "renderer/util/gl_index_buffers.h"

#include "platform/gl/gl.h"

namespace ark {

GLResourceManager::GLResourceManager(const sp<Dictionary<bitmap>>& bitmapLoader, const sp<Dictionary<bitmap>>& bitmapBoundsLoader)
    : _recycler(sp<GLRecycler>::make()), _gl_texture_loader(sp<GLTextureResource>::make(_recycler, bitmapLoader, bitmapBoundsLoader)), _tick(0)
{
}

GLResourceManager::~GLResourceManager()
{
}

void GLResourceManager::onSurfaceReady(GraphicsContext& graphicsContext)
{
    doRecycling(graphicsContext);
    doSurfaceReady(graphicsContext);
}

void GLResourceManager::onDrawFrame(GraphicsContext& graphicsContext)
{
    for(const PreparingGLResource& resource : _preparing_items.clear())
        if(!resource._resource.isExpired())
        {
            resource._resource.prepare(graphicsContext, resource._strategy == PS_ONCE_FORCE);
            if(resource._strategy == PS_ONCE_AND_ON_SURFACE_READY)
                _on_surface_ready_items.insert(resource._resource);
        }

    uint32_t m = (++_tick) % 301;
    if(m == 0)
        doRecycling(graphicsContext);
    else if (m == 150)
        _recycler->doRecycling(graphicsContext);
}

void GLResourceManager::prepare(const sp<GLResource>& resource, PreparingStrategy strategy)
{
    switch(strategy)
    {
    case PS_ONCE_AND_ON_SURFACE_READY:
    case PS_ONCE:
    case PS_ONCE_FORCE:
        _preparing_items.push(PreparingGLResource(resource, strategy));
        break;
    case PS_ON_SURFACE_READY:
        _on_surface_ready_items.insert(resource);
        break;
    }
}

void GLResourceManager::prepare(const GLBuffer& buffer, GLResourceManager::PreparingStrategy strategy)
{
    prepare(buffer._stub, strategy);
}

void GLResourceManager::recycle(const sp<GLResource>& resource) const
{
    _recycler->recycle(resource);
}

sp<GLTexture> GLResourceManager::loadGLTexture(const String& name)
{
    const sp<GLTexture> texture = _gl_texture_loader->get(name);
    DCHECK(texture, "Texture \"%s\" not loaded", name.c_str());
    prepare(texture, PS_ONCE_AND_ON_SURFACE_READY);
    return texture;
}

sp<GLTexture> GLResourceManager::createGLTexture(uint32_t width, uint32_t height, const sp<Variable<bitmap>>& bitmapVariable)
{
    sp<GLTexture> texture = sp<GLTexture>::make(_recycler, width, height, bitmapVariable);
    prepare(texture, PS_ONCE_AND_ON_SURFACE_READY);
    return texture;
}

GLBuffer GLResourceManager::makeGLBuffer(const sp<GLBuffer::Uploader>& uploader, GLenum type, GLenum usage)
{
    GLBuffer buffer(_recycler, uploader, type, usage);
    prepare(buffer, PS_ONCE_AND_ON_SURFACE_READY);
    return buffer;
}

GLBuffer GLResourceManager::makeDynamicArrayBuffer() const
{
    return GLBuffer(_recycler, nullptr, GL_ARRAY_BUFFER, GL_DYNAMIC_DRAW);
}

GLBuffer::Snapshot GLResourceManager::makeGLBufferSnapshot(GLBuffer::Name name, const GLBuffer::UploadMakerFunc& maker, size_t size)
{
    if(name == GLBuffer::NAME_NONE)
        return GLBuffer(_recycler, nullptr, GL_ELEMENT_ARRAY_BUFFER, GL_DYNAMIC_DRAW).snapshot(maker(size));

    GLBuffer& shared = _shared_buffers[name];
    if(!shared || shared.size() < size)
    {
        const sp<GLBuffer::Uploader> uploader = maker(size);
        NOT_NULL(uploader);
        shared = makeGLBuffer(uploader, GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW);
        prepare(shared, GLResourceManager::PS_ONCE_AND_ON_SURFACE_READY);
    }
    return shared.snapshot(size);
}

const sp<GLRecycler>& GLResourceManager::recycler() const
{
    return _recycler;
}

void GLResourceManager::doRecycling(GraphicsContext& graphicsContext)
{
    for(auto iter = _on_surface_ready_items.begin(); iter != _on_surface_ready_items.end();)
    {
        const ExpirableGLResource& resource = *iter;
        if(resource.isExpired())
        {
            resource.recycle(graphicsContext);
            iter = _on_surface_ready_items.erase(iter);
        }
        else
            ++iter;
    }
}

void GLResourceManager::doSurfaceReady(GraphicsContext& graphicsContext)
{
    for(const ExpirableGLResource& resource :_on_surface_ready_items)
        resource.recycle(graphicsContext);

    for(const ExpirableGLResource& resource :_on_surface_ready_items)
        resource.prepare(graphicsContext, true);
}

GLResourceManager::ExpirableGLResource::ExpirableGLResource(const sp<GLResource>& resource)
    : _resource(resource) {
}

GLResourceManager::ExpirableGLResource::ExpirableGLResource(const GLResourceManager::ExpirableGLResource& other)
    : _resource(other._resource) {
}

const sp<GLResource>& GLResourceManager::ExpirableGLResource::resource() const
{
    return _resource;
}

bool GLResourceManager::ExpirableGLResource::isExpired() const
{
    return _resource.unique();
}

void GLResourceManager::ExpirableGLResource::prepare(GraphicsContext& graphicsContext, bool force) const
{
    if(force || !_resource->id())
        _resource->prepare(graphicsContext);
}

void GLResourceManager::ExpirableGLResource::recycle(GraphicsContext& graphicsContext) const
{
    _resource->recycle(graphicsContext);
}

bool GLResourceManager::ExpirableGLResource::operator <(const GLResourceManager::ExpirableGLResource& other) const
{
    return _resource < other._resource;
}

GLResourceManager::PreparingGLResource::PreparingGLResource(const GLResourceManager::ExpirableGLResource& resource, GLResourceManager::PreparingStrategy strategy)
    : _resource(resource), _strategy(strategy)
{
}

GLResourceManager::PreparingGLResource::PreparingGLResource(const GLResourceManager::PreparingGLResource& other)
    : _resource(other._resource), _strategy(other._strategy)
{
}

bool GLResourceManager::PreparingGLResource::operator <(const GLResourceManager::PreparingGLResource& other) const
{
    return _resource < other._resource;
}

}
