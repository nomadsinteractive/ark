#include "renderer/base/render_controller.h"

#include "core/inf/runnable.h"
#include "core/util/log.h"

#include "graphics/base/bitmap.h"
#include "graphics/base/size.h"

#include "renderer/base/framebuffer.h"
#include "renderer/base/graphics_context.h"
#include "renderer/base/recycler.h"
#include "renderer/base/render_engine.h"
#include "renderer/inf/renderer_factory.h"

#include "platform/platform.h"

namespace ark {

namespace {

class UploaderBitmapBundle : public Texture::Uploader {
public:
    UploaderBitmapBundle(const sp<Dictionary<bitmap>>& bitmapLoader, const String& name)
        : _bitmap_loader(bitmapLoader), _name(name) {
    }

    virtual void upload(GraphicsContext& graphicContext, Texture::Delegate& delegate) override {
        const bitmap bitmap = _bitmap_loader->get(_name);
        DCHECK(bitmap, "Texture resource \"%s\" not found", _name.c_str());
        delegate.uploadBitmap(graphicContext, 0, bitmap);
    }

private:
    sp<Dictionary<bitmap>> _bitmap_loader;
    String _name;
};


class GLTextureBundle : public Dictionary<sp<Texture>> {
public:
    GLTextureBundle(const sp<RendererFactory>& rendererFactory, const sp<Dictionary<bitmap>>& bitmapLoader, const sp<Dictionary<bitmap>>& bitmapBoundsLoader)
        : _renderer_factory(rendererFactory), _bitmap_loader(bitmapLoader), _bitmap_bounds_loader(bitmapBoundsLoader)
    {
    }

    virtual sp<Texture> get(const String& name) override {
        const bitmap bitmapBounds = _bitmap_bounds_loader->get(name);
        DCHECK(bitmapBounds, "Texture resource \"%s\" not found", name.c_str());
        return _renderer_factory->createTexture(sp<Size>::make(bitmapBounds->width(), bitmapBounds->height()), Texture::TYPE_2D, sp<UploaderBitmapBundle>::make(_bitmap_loader, name));
    }

private:
    sp<RendererFactory> _renderer_factory;

    sp<Dictionary<bitmap>> _bitmap_loader;
    sp<Dictionary<bitmap>> _bitmap_bounds_loader;
};

}

RenderController::RenderController(const sp<RenderEngine>& renderEngine, const sp<Recycler>& recycler, const sp<Dictionary<bitmap>>& bitmapLoader, const sp<Dictionary<bitmap>>& bitmapBoundsLoader)
    : _render_engine(renderEngine), _recycler(recycler), _bitmap_loader(bitmapLoader), _bitmap_bounds_loader(bitmapBoundsLoader)
{
    _named_buffers[NamedBuffer::NAME_QUADS] = NamedBuffer::Quads::make(*this);
    _named_buffers[NamedBuffer::NAME_NINE_PATCH] = NamedBuffer::NinePatch::make(*this);
    _named_buffers[NamedBuffer::NAME_POINTS] = NamedBuffer::Points::make(*this);
}

void RenderController::reset()
{
    DTHREAD_CHECK(THREAD_ID_CORE);
    for(const sp<NamedBuffer>& i : _named_buffers)
        i->reset();
}

void RenderController::onSurfaceReady(GraphicsContext& graphicsContext)
{
    doRecycling(graphicsContext);
    doSurfaceReady(graphicsContext);
}

void RenderController::prepare(GraphicsContext& graphicsContext, LFQueue<PreparingResource>& items)
{
    PreparingResource front;
    while(items.pop(front)) {
        if(!front._resource.isExpired() || front._strategy == RenderController::US_RELOAD)
        {
            if(front._strategy == RenderController::US_RELOAD && front._resource.resource()->id() != 0)
                front._resource.recycle(graphicsContext);

            front._resource.upload(graphicsContext);
            if(front._strategy == RenderController::US_ONCE_AND_ON_SURFACE_READY)
                _on_surface_ready_items.insert(front._resource);
        }
    }
}

void RenderController::onDrawFrame(GraphicsContext& graphicsContext)
{
    prepare(graphicsContext, _preparing_items);

    uint32_t tick = graphicsContext.tick();
    if(tick == 0)
        doRecycling(graphicsContext);
    else if (tick == 150)
        _recycler->doRecycling(graphicsContext);
}

void RenderController::upload(const sp<Resource>& resource, const sp<Uploader>& uploader, RenderController::UploadStrategy strategy)
{
    switch(strategy & 3)
    {
    case RenderController::US_ONCE_AND_ON_SURFACE_READY:
    case RenderController::US_ONCE:
    case RenderController::US_RELOAD:
        _preparing_items.push(PreparingResource(ExpirableResource(resource, uploader), strategy));
        break;
    case RenderController::US_ON_SURFACE_READY:
        _on_surface_ready_items.insert(ExpirableResource(resource, uploader));
        break;
    }
}

void RenderController::uploadBuffer(const Buffer& buffer, const sp<Uploader>& uploader, RenderController::UploadStrategy strategy)
{
    upload(buffer._delegate, uploader, strategy);
}

const sp<Recycler>& RenderController::recycler() const
{
    return _recycler;
}

void RenderController::doRecycling(GraphicsContext& graphicsContext)
{
    for(auto iter = _on_surface_ready_items.begin(); iter != _on_surface_ready_items.end();)
    {
        const ExpirableResource& resource = *iter;
        if(resource.isExpired())
        {
            resource.recycle(graphicsContext);
            iter = _on_surface_ready_items.erase(iter);
        }
        else
            ++iter;
    }
}

void RenderController::doSurfaceReady(GraphicsContext& graphicsContext)
{
    for(const ExpirableResource& resource : _on_surface_ready_items)
        resource.recycle(graphicsContext);

    for(const ExpirableResource& resource : _on_surface_ready_items)
        resource.upload(graphicsContext);
}

const sp<RenderEngine>& RenderController::renderEngine() const
{
    return _render_engine;
}

sp<PipelineFactory> RenderController::createPipelineFactory() const
{
    return _render_engine->rendererFactory()->createPipelineFactory();
}

sp<Dictionary<sp<Texture>>> RenderController::createTextureBundle() const
{
    return sp<GLTextureBundle>::make(_render_engine->rendererFactory(), _bitmap_loader, _bitmap_bounds_loader);
}

sp<Texture> RenderController::createTexture(const sp<Size>& size, Texture::Type type, const sp<Texture::Uploader>& uploader, RenderController::UploadStrategy us)
{
    const sp<Texture> texture = _render_engine->rendererFactory()->createTexture(size, type, uploader);
    upload(texture, nullptr, us);
    return texture;
}

sp<Texture> RenderController::createTexture2D(const sp<Size>& size, const sp<Texture::Uploader>& uploader, RenderController::UploadStrategy us)
{
    return createTexture(size, Texture::TYPE_2D, uploader, us);
}

Buffer RenderController::makeVertexBuffer(Buffer::Usage usage, const sp<Uploader>& uploader)
{
    return makeBuffer(Buffer::TYPE_VERTEX, usage, uploader);
}

Buffer RenderController::makeIndexBuffer(Buffer::Usage usage, const sp<Uploader>& uploader)
{
    return makeBuffer(Buffer::TYPE_INDEX, usage, uploader);
}

sp<Framebuffer> RenderController::makeFramebuffer(const sp<Renderer>& renderer, const sp<Texture>& texture)
{
    const sp<Framebuffer> framebuffer = renderEngine()->rendererFactory()->createFramebuffer(renderer, texture);
    upload(framebuffer->resource(), nullptr, RenderController::US_ONCE_AND_ON_SURFACE_READY);
    return framebuffer;
}

sp<Boolean> RenderController::makeSynchronizeFlag()
{
    const sp<SynchronizeFlag> flag = sp<SynchronizeFlag>::make();
    _synchronize_flags.push_back(flag);
    return flag;
}

Buffer RenderController::makeBuffer(Buffer::Type type, Buffer::Usage usage, const sp<Uploader>& uploader)
{
    Buffer buffer(_render_engine->rendererFactory()->createBuffer(type, usage));
    uploadBuffer(buffer, uploader, uploader ? RenderController::US_ONCE_AND_ON_SURFACE_READY : RenderController::US_ON_SURFACE_READY);
    return buffer;
}

void RenderController::addPreUpdateRequest(const sp<Runnable>& task, const sp<Boolean>& expired)
{
    if(expired)
        _on_pre_update_request.push_back(task, expired);
    else
    {
        const sp<Disposed> e = task.as<Disposed>();
        DCHECK(e, "Adding an undisposable running task, it's that what you REALLY want?");
        _on_pre_update_request.push_back(task, e);
    }
}

void RenderController::preUpdate()
{
#ifdef ARK_FLAG_DEBUG
    static int i = 0;
    if(i ++ == 600)
    {
        LOGD("_on_pre_update_request: %d", _on_pre_update_request.items().size());
        i = 0;
    }
#endif
    _defered_instances.clear();

    for(const sp<SynchronizeFlag>& i : _synchronize_flags)
        i->reset();

    for(const sp<Runnable>& runnable : _on_pre_update_request)
        runnable->run();
}

void RenderController::deferUnref(const Box& box)
{
    _defered_instances.push_back(box);
}

const sp<NamedBuffer>& RenderController::getNamedBuffer(NamedBuffer::Name name) const
{
    DTHREAD_CHECK(THREAD_ID_CORE);
    return _named_buffers[name];
}

RenderController::ExpirableResource::ExpirableResource(const sp<Resource>& resource, const sp<Uploader>& uploader)
    : _resource(resource), _uploader(uploader)
{
}

const sp<Resource>& RenderController::ExpirableResource::resource() const
{
    return _resource;
}

bool RenderController::ExpirableResource::isExpired() const
{
    return _resource.unique();
}

void RenderController::ExpirableResource::upload(GraphicsContext& graphicsContext) const
{
    _resource->upload(graphicsContext, _uploader);
}

void RenderController::ExpirableResource::recycle(GraphicsContext& graphicsContext) const
{
    _resource->recycle()(graphicsContext);
}

bool RenderController::ExpirableResource::operator <(const RenderController::ExpirableResource& other) const
{
    return _resource < other._resource;
}

RenderController::PreparingResource::PreparingResource(const RenderController::ExpirableResource& resource, RenderController::UploadStrategy strategy)
    : _resource(resource), _strategy(strategy)
{
}

bool RenderController::PreparingResource::operator <(const RenderController::PreparingResource& other) const
{
    return _resource < other._resource;
}

RenderController::SynchronizeFlag::SynchronizeFlag()
    : _value(false)
{
}

bool RenderController::SynchronizeFlag::val()
{
    if(_value)
    {
        _value = false;
        return true;
    }
    return false;
}

void RenderController::SynchronizeFlag::reset()
{
    _value = true;
}

}
