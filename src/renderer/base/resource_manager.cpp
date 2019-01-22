#include "renderer/base/resource_manager.h"

#include "core/inf/array.h"
#include "core/inf/variable.h"
#include "core/util/log.h"

#include "graphics/base/bitmap.h"
#include "graphics/base/size.h"

#include "renderer/base/buffer.h"
#include "renderer/base/graphics_context.h"
#include "renderer/base/render_context.h"
#include "renderer/base/recycler.h"

namespace ark {

ResourceManager::ResourceManager(const sp<Dictionary<bitmap>>& bitmapLoader, const sp<Dictionary<bitmap>>& bitmapBoundsLoader)
    : _bitmap_loader(bitmapLoader), _bitmap_bounds_loader(bitmapBoundsLoader), _recycler(sp<Recycler>::make())
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

void ResourceManager::prepare(GraphicsContext& graphicsContext, LFStack<PreparingGLResource>& items)
{
    for(const PreparingGLResource& i : items.clear())
        if(!i._resource.isExpired() || i._strategy == US_RELOAD)
        {
            if(i._strategy == US_RELOAD && i._resource.resource()->id() != 0)
                i._resource.recycle(graphicsContext);

            i._resource.upload(graphicsContext);
            if(i._strategy == US_ONCE_AND_ON_SURFACE_READY)
                _on_surface_ready_items.insert(i._resource);
        }
}

void ResourceManager::onDrawFrame(GraphicsContext& graphicsContext)
{
    prepare(graphicsContext, _preparing_items[1]);
    prepare(graphicsContext, _preparing_items[0]);

    uint32_t tick = graphicsContext.tick();
    if(tick == 0)
        doRecycling(graphicsContext);
    else if (tick == 150)
        _recycler->doRecycling(graphicsContext);
}

void ResourceManager::upload(const sp<Resource>& resource, const sp<Uploader>& uploader, UploadStrategy strategy)
{
    switch(strategy & 3)
    {
    case US_ONCE_AND_ON_SURFACE_READY:
    case US_ONCE:
    case US_RELOAD:
        _preparing_items[strategy & US_PRIORITY_HIGHT ? 1 : 0].push(PreparingGLResource(ExpirableGLResource(resource, uploader), strategy));
        break;
    case US_ON_SURFACE_READY:
        _on_surface_ready_items.insert(ExpirableGLResource(resource, uploader));
        break;
    }
}

void ResourceManager::uploadBuffer(const Buffer& buffer, const sp<Uploader>& uploader, UploadStrategy strategy)
{
    upload(buffer._delegate, uploader, strategy);
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
        resource.upload(graphicsContext);
}

ResourceManager::ExpirableGLResource::ExpirableGLResource(const sp<Resource>& resource, const sp<Uploader>& uploader)
    : _resource(resource), _uploader(uploader)
{
}

const sp<Resource>& ResourceManager::ExpirableGLResource::resource() const
{
    return _resource;
}

bool ResourceManager::ExpirableGLResource::isExpired() const
{
    return _resource.unique();
}

void ResourceManager::ExpirableGLResource::upload(GraphicsContext& graphicsContext) const
{
    _resource->upload(graphicsContext, _uploader);
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

bool ResourceManager::PreparingGLResource::operator <(const ResourceManager::PreparingGLResource& other) const
{
    return _resource < other._resource;
}

}
