#include "renderer/base/render_controller.h"

#include "core/base/manifest.h"
#include "core/inf/runnable.h"
#include "core/util/log.h"

#include "graphics/base/bitmap.h"
#include "graphics/base/size.h"

#include "renderer/base/framebuffer.h"
#include "renderer/base/graphics_context.h"
#include "renderer/base/model.h"
#include "renderer/base/recycler.h"
#include "renderer/base/render_engine.h"
#include "renderer/inf/renderer_factory.h"
#include "renderer/inf/vertices.h"
#include "renderer/util/element_util.h"
#include "renderer/util/named_buffer.h"

#include "platform/platform.h"

namespace ark {

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
        if(i)
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

void RenderController::upload(const sp<Resource>& resource, const sp<Uploader>& uploader, RenderController::UploadStrategy strategy, RenderController::UploadPriority priority)
{
    switch(strategy & 3)
    {
    case RenderController::US_ONCE_AND_ON_SURFACE_READY:
    case RenderController::US_ONCE:
    case RenderController::US_RELOAD:
        _preparing_items.push(PreparingResource(RenderResource(resource, uploader, priority), strategy));
        break;
    case RenderController::US_ON_SURFACE_READY:
        _on_surface_ready_items.insert(RenderResource(resource, uploader, priority));
        break;
    }
}

void RenderController::uploadBuffer(const Buffer& buffer, const sp<Uploader>& uploader, RenderController::UploadStrategy strategy, RenderController::UploadPriority priority)
{
    upload(buffer._delegate, uploader, strategy, priority);
}

const sp<Recycler>& RenderController::recycler() const
{
    return _recycler;
}

void RenderController::doRecycling(GraphicsContext& graphicsContext)
{
    for(auto iter = _on_surface_ready_items.begin(); iter != _on_surface_ready_items.end();)
    {
        const RenderResource& resource = *iter;
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
    for(const RenderResource& resource : _on_surface_ready_items)
        resource.recycle(graphicsContext);

    for(const RenderResource& resource : _on_surface_ready_items)
        if(resource.uploadPriority() == UP_LEVEL_2)
            resource.upload(graphicsContext);

    for(const RenderResource& resource : _on_surface_ready_items)
        if(resource.uploadPriority() != UP_LEVEL_2)
            resource.upload(graphicsContext);
}

const sp<RenderEngine>& RenderController::renderEngine() const
{
    return _render_engine;
}

sp<Camera::Delegate> RenderController::createCamera() const
{
    return _render_engine->rendererFactory()->createCamera(Ark::instance().manifest()->renderer()._coordinate_system);
}

sp<PipelineFactory> RenderController::createPipelineFactory() const
{
    return _render_engine->rendererFactory()->createPipelineFactory();
}

sp<Texture> RenderController::createTexture(const sp<Size>& size, const sp<Texture::Parameters>& parameters, const sp<Texture::Uploader>& uploader, RenderController::UploadStrategy us)
{
    const sp<Texture> texture = _render_engine->rendererFactory()->createTexture(size, parameters, uploader);
    upload(texture, nullptr, us);
    return texture;
}

sp<Texture> RenderController::createTexture2D(const sp<Size>& size, const sp<Texture::Uploader>& uploader, RenderController::UploadStrategy us)
{
    return createTexture(size, sp<Texture::Parameters>::make(Texture::TYPE_2D), uploader, us);
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

void RenderController::deferUnref(Box box)
{
    _defered_instances.push_back(std::move(box));
}

const sp<NamedBuffer>& RenderController::getNamedBuffer(NamedBuffer::Name name) const
{
    DTHREAD_CHECK(THREAD_ID_CORE);
    return _named_buffers[name];
}

sp<NamedBuffer> RenderController::getSharedBuffer(RenderModel::Mode renderMode, const Model& model)
{
    const array<element_index_t>& indices = model.indices();
    element_index_t hash = ElementUtil::hash(indices);
    const auto iter = _shared_buffers.find(hash);
    if(iter != _shared_buffers.end())
        return iter->second;

    bool degenerate = renderMode == RenderModel::RENDER_MODE_TRIANGLE_STRIP;
    size_t indexCount = indices->length();
    size_t vertexCount = model.vertices()->length();

    sp<NamedBuffer> sharedBuffer;
    if(degenerate)
        sharedBuffer = sp<NamedBuffer>::make(makeIndexBuffer(Buffer::USAGE_STATIC),
                                             [indices, vertexCount](size_t objectCount)->sp<Uploader> { return sp<NamedBuffer::Degenerate>::make(objectCount, vertexCount, indices); },
                                             [indexCount](size_t objectCount)->size_t { return ((indexCount + 2) * objectCount - 2) * sizeof(element_index_t); });
    else
        sharedBuffer = sp<NamedBuffer>::make(makeIndexBuffer(Buffer::USAGE_STATIC),
                                             [indices, vertexCount](size_t objectCount)->sp<Uploader> { return sp<NamedBuffer::Concat>::make(objectCount, vertexCount, indices); },
                                             [indexCount](size_t objectCount)->size_t { return indexCount * objectCount * sizeof(element_index_t); });
    _shared_buffers.insert(std::make_pair(hash, sharedBuffer));
    return sharedBuffer;
}

RenderController::RenderResource::RenderResource(const sp<Resource>& resource, const sp<Uploader>& uploader, UploadPriority uploadPriority)
    : _resource(resource), _uploader(uploader), _upload_priority(uploadPriority)
{
}

const sp<Resource>& RenderController::RenderResource::resource() const
{
    return _resource;
}

bool RenderController::RenderResource::isExpired() const
{
    return _resource.unique();
}

void RenderController::RenderResource::upload(GraphicsContext& graphicsContext) const
{
    _resource->upload(graphicsContext, _uploader);
}

void RenderController::RenderResource::recycle(GraphicsContext& graphicsContext) const
{
    _resource->recycle()(graphicsContext);
}

RenderController::UploadPriority RenderController::RenderResource::uploadPriority() const
{
    return _upload_priority;
}

bool RenderController::RenderResource::operator <(const RenderController::RenderResource& other) const
{
    return _resource < other._resource;
}

RenderController::PreparingResource::PreparingResource(const RenderController::RenderResource& resource, RenderController::UploadStrategy strategy)
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

bool RenderController::SynchronizeFlag::update(uint64_t /*timestamp*/)
{
    return true;
}

void RenderController::SynchronizeFlag::reset()
{
    _value = true;
}

}
