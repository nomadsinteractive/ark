#include "renderer/base/resource_manager.h"

#include "core/inf/array.h"
#include "core/inf/variable.h"
#include "core/impl/array/fixed_array.h"
#include "core/impl/array/dynamic_array.h"
#include "core/types/weak_ptr.h"
#include "core/util/log.h"

#include "graphics/base/bitmap.h"
#include "graphics/base/size.h"

#include "renderer/base/buffer.h"
#include "renderer/base/gl_context.h"
#include "renderer/base/recycler.h"
#include "renderer/base/gl_snippet_delegate.h"
#include "renderer/base/graphics_context.h"
#include "renderer/base/shader.h"
#include "renderer/inf/gl_snippet.h"

#include "renderer/opengl/base/gl_texture_2d.h"
#include "renderer/opengl/util/gl_index_buffers.h"

#include "platform/gl/gl.h"

namespace ark {

namespace {

class GLTextureResource : public Dictionary<sp<Texture>> {
public:
    GLTextureResource(const sp<Recycler>& recycler, const sp<Dictionary<bitmap>>& bitmapLoader, const sp<Dictionary<bitmap>>& bitmapBoundsLoader)
        : _recycler(recycler), _bitmap_loader(bitmapLoader), _bitmap_bounds_loader(bitmapBoundsLoader)
    {
    }

    virtual sp<Texture> get(const String& name) override {
        const bitmap bitmapBounds = _bitmap_bounds_loader->get(name);
        DCHECK(bitmapBounds, "Texture resource \"%s\" not found", name.c_str());
        const sp<Texture::Parameters> params = sp<Texture::Parameters>::make();
        const sp<Size> size = sp<Size>::make(static_cast<float>(bitmapBounds->width()), static_cast<float>(bitmapBounds->height()));
        const sp<GLTexture2D> texture = sp<GLTexture2D>::make(_recycler, size, params, sp<Variable<bitmap>::Get>::make(_bitmap_loader, name));
        return sp<Texture>::make(size, texture);
    }

private:
    sp<Recycler> _recycler;

    sp<Dictionary<bitmap>> _bitmap_loader;
    sp<Dictionary<bitmap>> _bitmap_bounds_loader;
};

}

ResourceManager::ResourceManager(const sp<Dictionary<bitmap>>& bitmapLoader, const sp<Dictionary<bitmap>>& bitmapBoundsLoader)
    : _bitmap_loader(bitmapLoader), _bitmap_bounds_loader(bitmapBoundsLoader),
      _recycler(sp<Recycler>::make()), _gl_texture_loader(sp<GLTextureResource>::make(_recycler, bitmapLoader, bitmapBoundsLoader)), _tick(0)
{
}

ResourceManager::~ResourceManager()
{
}

const sp<Dictionary<bitmap>>& ResourceManager::bitmapLoader() const
{
    return _bitmap_loader;
}

const sp<Dictionary<bitmap>>& ResourceManager::bitmapBoundsLoader() const
{
    return _bitmap_bounds_loader;
}

void ResourceManager::onSurfaceReady(GraphicsContext& graphicsContext)
{
    doRecycling(graphicsContext);
    doSurfaceReady(graphicsContext);
}

void ResourceManager::onDrawFrame(GraphicsContext& graphicsContext)
{
    for(const PreparingGLResource& i : _preparing_items.clear())
        if(!i._resource.isExpired())
        {
            if(i._strategy == US_ONCE_FORCE && i._resource.resource()->id() != 0)
                i._resource.recycle(graphicsContext);

            i._resource.prepare(graphicsContext);
            if(i._strategy == US_ONCE_AND_ON_SURFACE_READY)
                _on_surface_ready_items.insert(i._resource);
        }

    uint32_t m = (++_tick) % 301;
    if(m == 0)
        doRecycling(graphicsContext);
    else if (m == 150)
        _recycler->doRecycling(graphicsContext);
}

void ResourceManager::upload(const sp<Resource>& resource, UploadStrategy strategy)
{
    switch(strategy)
    {
    case US_ONCE_AND_ON_SURFACE_READY:
    case US_ONCE:
    case US_ONCE_FORCE:
        _preparing_items.push(PreparingGLResource(resource, strategy));
        break;
    case US_ON_SURFACE_READY:
        _on_surface_ready_items.insert(resource);
        break;
    }
}

void ResourceManager::prepare(const Buffer& buffer, ResourceManager::UploadStrategy strategy)
{
    upload(buffer._delegate, strategy);
}

void ResourceManager::recycle(const sp<Resource>& resource) const
{
    _recycler->recycle(resource);
}

sp<Texture> ResourceManager::loadGLTexture(const String& name)
{
    const sp<Texture> texture = _gl_texture_loader->get(name);
    DCHECK(texture, "Texture \"%s\" not loaded", name.c_str());
    upload(texture, US_ONCE_AND_ON_SURFACE_READY);
    return texture;
}

sp<Texture> ResourceManager::createGLTexture(uint32_t width, uint32_t height, const sp<Variable<bitmap>>& bitmapVariable, UploadStrategy ps)
{
    const sp<Size> size = sp<Size>::make(static_cast<float>(width), static_cast<float>(height));
    const sp<GLTexture2D> texture2d = sp<GLTexture2D>::make(_recycler, size, sp<Texture::Parameters>::make(), bitmapVariable);
    const sp<Texture> texture = sp<Texture>::make(size, texture2d);
    upload(texture, ps);
    return texture;
}

//Buffer ResourceManager::makeBuffer(const sp<Buffer::Uploader>& uploader, Buffer::Type type, Buffer::Usage usage)
//{
//    Buffer buffer(_recycler, uploader, type, usage);
//    prepare(buffer, US_ONCE_AND_ON_SURFACE_READY);
//    return buffer;
//}

//Buffer ResourceManager::makeDynamicArrayBuffer() const
//{
//    return Buffer(_recycler, nullptr, Buffer::TYPE_VERTEX, Buffer::USAGE_DYNAMIC);
//}

//Buffer::Snapshot ResourceManager::makeBufferSnapshot(Buffer::Name name, const Buffer::UploadMakerFunc& maker, size_t reservedObjectCount, size_t size)
//{
//    if(name == Buffer::NAME_NONE)
//        return Buffer(_recycler, nullptr, Buffer::TYPE_INDEX, Buffer::USAGE_DYNAMIC).snapshot(maker(size));

//    sp<SharedBuffer> sb;
//    if(!_shared_buffers.pop(sb))
//        sb = sp<SharedBuffer>::make();

//    Buffer& shared = sb->_buffers[name];
//    if(!shared || shared.size() < size)
//    {
//        const sp<Buffer::Uploader> uploader = maker(reservedObjectCount);
//        DCHECK(uploader && uploader->size() >= size, "Making GLBuffer::Uploader failed, object-count: %d, uploader-size: %d, required-size: %d", reservedObjectCount, uploader ? uploader->size() : 0, size);
//        shared = makeBuffer(uploader, Buffer::TYPE_INDEX, Buffer::USAGE_STATIC);
//    }
//    _shared_buffers.push(sb);

//    return shared.snapshot(size);
//}

const sp<Recycler>& ResourceManager::recycler() const
{
    return _recycler;
}

void ResourceManager::doRecycling(GraphicsContext& graphicsContext)
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

void ResourceManager::doSurfaceReady(GraphicsContext& graphicsContext)
{
    for(const ExpirableGLResource& resource : _on_surface_ready_items)
        resource.recycle(graphicsContext);

    for(const ExpirableGLResource& resource : _on_surface_ready_items)
        resource.prepare(graphicsContext);
}

ResourceManager::ExpirableGLResource::ExpirableGLResource(const sp<Resource>& resource)
    : _resource(resource) {
}

ResourceManager::ExpirableGLResource::ExpirableGLResource(const ResourceManager::ExpirableGLResource& other)
    : _resource(other._resource) {
}

const sp<Resource>& ResourceManager::ExpirableGLResource::resource() const
{
    return _resource;
}

bool ResourceManager::ExpirableGLResource::isExpired() const
{
    return _resource.unique();
}

void ResourceManager::ExpirableGLResource::prepare(GraphicsContext& graphicsContext) const
{
    if(_resource->id() == 0)
        _resource->upload(graphicsContext);
}

void ResourceManager::ExpirableGLResource::recycle(GraphicsContext& graphicsContext) const
{
    _resource->recycle()(graphicsContext);
}

bool ResourceManager::ExpirableGLResource::operator <(const ResourceManager::ExpirableGLResource& other) const
{
    return _resource < other._resource;
}

ResourceManager::PreparingGLResource::PreparingGLResource(const ResourceManager::ExpirableGLResource& resource, ResourceManager::UploadStrategy strategy)
    : _resource(resource), _strategy(strategy)
{
}

ResourceManager::PreparingGLResource::PreparingGLResource(const ResourceManager::PreparingGLResource& other)
    : _resource(other._resource), _strategy(other._strategy)
{
}

bool ResourceManager::PreparingGLResource::operator <(const ResourceManager::PreparingGLResource& other) const
{
    return _resource < other._resource;
}

}
