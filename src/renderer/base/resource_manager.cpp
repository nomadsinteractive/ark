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
#include "renderer/base/snippet_delegate.h"
#include "renderer/base/graphics_context.h"
#include "renderer/base/recycler.h"
#include "renderer/base/shader.h"
#include "renderer/inf/snippet.h"

namespace ark {

ResourceManager::ResourceManager(const sp<Dictionary<bitmap>>& bitmapLoader, const sp<Dictionary<bitmap>>& bitmapBoundsLoader)
    : _bitmap_loader(bitmapLoader), _bitmap_bounds_loader(bitmapBoundsLoader), _recycler(sp<Recycler>::make()), _tick(0)
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

void ResourceManager::uploadBuffer(const Buffer& buffer, UploadStrategy strategy)
{
    upload(buffer._delegate, strategy);
}

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
