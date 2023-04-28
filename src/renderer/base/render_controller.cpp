#include "renderer/base/render_controller.h"

#include "core/base/future.h"
#include "core/base/enums.h"
#include "core/inf/runnable.h"
#include "core/inf/writable.h"
#include "core/impl/uploader/uploader_snapshot.h"
#include "core/util/boolean_type.h"
#include "core/util/log.h"

#include "graphics/base/bitmap.h"
#include "graphics/base/size.h"

#include "renderer/base/framebuffer.h"
#include "renderer/base/graphics_context.h"
#include "renderer/base/model.h"
#include "renderer/base/recycler.h"
#include "renderer/base/render_engine.h"
#include "renderer/base/render_engine_context.h"
#include "renderer/base/shared_indices.h"
#include "renderer/inf/renderer_factory.h"
#include "renderer/inf/snippet_factory.h"
#include "renderer/inf/vertices.h"
#include "renderer/util/render_util.h"

#include "app/base/application_manifest.h"

namespace ark {

namespace {

class WritableIndice : public Writable {
public:
    WritableIndice(size_t size)
        : _hash(0), _indices(size) {
    }

    virtual uint32_t write(const void* buffer, uint32_t size, uint32_t offset) override {
        _hash = _hash * 101 + RenderUtil::hash(reinterpret_cast<const element_index_t*>(buffer), size / sizeof(element_index_t));
        memcpy(_indices.data() + offset, buffer, size);
        return size;
    }

    uint32_t _hash;
    std::vector<element_index_t> _indices;
};

class ResourceUploadBufferSnapshot : public Resource {
public:
    ResourceUploadBufferSnapshot(sp<Buffer::Delegate> buffer, Uploader& input)
        : _buffer(std::move(buffer)), _input_snapshot(input) {
    }

    virtual uint64_t id() override {
        return _buffer->id();
    }

    virtual void upload(GraphicsContext& graphicsContext) override {
        _buffer->uploadBuffer(graphicsContext, _input_snapshot);
    }

    virtual ResourceRecycleFunc recycle() override {
        return _buffer->recycle();
    }

private:
    sp<Buffer::Delegate> _buffer;
    InputSnapshot _input_snapshot;
};

class BufferUpdatable : public Updatable {
public:
    BufferUpdatable(RenderController& renderController, sp<Uploader> input, sp<Buffer::Delegate> buffer)
        : _render_controller(renderController), _buffer(std::move(buffer)), _input(std::move(input)) {
    }

    virtual bool update(uint64_t timestamp) override {
        bool dirty = _input->update(timestamp) || _buffer->id() == 0;
        if(dirty)
            _render_controller.upload(sp<ResourceUploadBufferSnapshot>::make(_buffer, _input), RenderController::US_ONCE);
        return dirty;
    }

private:
    RenderController& _render_controller;
    sp<Buffer::Delegate> _buffer;
    sp<Uploader> _input;
};

}

RenderController::RenderController(const sp<RenderEngine>& renderEngine, const sp<Recycler>& recycler, const sp<Dictionary<bitmap>>& bitmapLoader, const sp<Dictionary<bitmap>>& bitmapBoundsLoader)
    : _render_engine(renderEngine), _recycler(recycler), _bitmap_loader(bitmapLoader), _bitmap_bounds_loader(bitmapBoundsLoader), _clock(Platform::getSteadyClock()), _gba(*this)
{
}

void RenderController::reset()
{
    DTHREAD_CHECK(THREAD_ID_CORE);
    _shared_indices.clear();
}

void RenderController::onSurfaceReady(GraphicsContext& graphicsContext)
{
    _on_surface_ready.foreach(graphicsContext, true, true);
}

void RenderController::prepare(GraphicsContext& graphicsContext, LFQueue<UploadingRenderResource>& items)
{
    UploadingRenderResource front;
    while(items.pop(front)) {
        if(!front._resource.isCancelled())
        {
            if(front._strategy == RenderController::US_RELOAD && front._resource.id() != 0)
                front._resource.recycle(graphicsContext);

            if((front._strategy & US_ONCE) || front._strategy == US_RELOAD)
                front._resource.upload(graphicsContext);

            if(front._strategy & RenderController::US_ON_EVERY_FRAME)
                _on_every_frame.append(front._priority, std::move(front._resource));
            else if(front._strategy & RenderController::US_ON_SURFACE_READY)
                _on_surface_ready.append(front._priority, std::move(front._resource));
        }
    }
}

void RenderController::onDrawFrame(GraphicsContext& graphicsContext)
{
    prepare(graphicsContext, _uploading_resources);
    _on_every_frame.foreach(graphicsContext, false, true);

    uint32_t tick = graphicsContext.tick() % 300;
    if(tick == 0)
        _on_surface_ready.foreach(graphicsContext, false, false);
    else if (tick == 150)
        _recycler->doRecycling(graphicsContext);

    for(const sp<Runnable>& runnable : _on_pre_render_runnable.update(0))
        runnable->run();
}

void RenderController::upload(sp<Resource> resource, UploadStrategy strategy, sp<Updatable> updatable, sp<Future> future, UploadPriority priority)
{
    if(strategy & RenderController::US_ON_CHANGE)
    {
        CHECK(updatable, "An updatable must be specified using \"on_change\" upload strategy");
        sp<Boolean> disposed = future ? future->canceled() : sp<Boolean>::make<BooleanByWeakRef<Resource>>(resource, 1);
        addPreComposeUpdatable(std::move(updatable), std::move(disposed));
    }
    if(strategy != RenderController::US_ON_CHANGE)
        _uploading_resources.push(UploadingRenderResource(RenderResource(std::move(resource), std::move(future)), strategy, priority));
}

void RenderController::uploadBuffer(Buffer& buffer, sp<Uploader> uploader, RenderController::UploadStrategy strategy, sp<Future> future, RenderController::UploadPriority priority)
{
    ASSERT(uploader);
    if(!future)
        future = sp<Future>::make(sp<BooleanByWeakRef<Buffer::Delegate>>::make(buffer.delegate(), 1));
    sp<Updatable> updatable = strategy & RenderController::US_ON_CHANGE ? sp<Updatable>::make<BufferUpdatable>(*this, uploader, buffer.delegate()) : nullptr;
    buffer.delegate()->setSize(uploader->size());
    //TODO: make this mess a bit more cleaner
    if(strategy == RenderController::US_ON_CHANGE)
        upload(nullptr, strategy, std::move(updatable), std::move(future), priority);
    else
        upload(sp<ResourceUploadBufferSnapshot>::make(buffer.delegate(), std::move(uploader)), strategy, std::move(updatable), std::move(future), priority);
}

const sp<Recycler>& RenderController::recycler() const
{
    return _recycler;
}

const sp<RenderEngine>& RenderController::renderEngine() const
{
    return _render_engine;
}

Camera RenderController::createCamera(Ark::RendererCoordinateSystem coordinateSystem) const
{
    Ark::RendererCoordinateSystem cs = coordinateSystem == Ark::COORDINATE_SYSTEM_DEFAULT ? _render_engine->context()->coordinateSystem() : coordinateSystem;
    return Camera(cs, _render_engine->rendererFactory()->createCamera(cs));
}

sp<PipelineFactory> RenderController::createPipelineFactory() const
{
    return _render_engine->rendererFactory()->createPipelineFactory();
}

sp<Texture> RenderController::createTexture(sp<Size> size, sp<Texture::Parameters> parameters, sp<Texture::Uploader> uploader, RenderController::UploadStrategy us, sp<Future> future)
{
    sp<Texture::Delegate> delegate = _render_engine->rendererFactory()->createTexture(size, parameters);
    DCHECK(delegate, "Unsupported TextureType: %d", parameters->_type);
    const sp<Texture> texture = sp<Texture>::make(std::move(delegate), std::move(size), std::move(uploader), std::move(parameters));
    upload(texture, us, nullptr, std::move(future));
    return texture;
}

sp<Texture> RenderController::createTexture2d(sp<Bitmap> bitmap, Texture::Format format, UploadStrategy us, sp<Future> future)
{
    sp<Size> size = sp<Size>::make(static_cast<float>(bitmap->width()), static_cast<float>(bitmap->height()));
    return createTexture(std::move(size), sp<Texture::Parameters>::make(Texture::TYPE_2D, nullptr, format), sp<Texture::UploaderBitmap>::make(std::move(bitmap)), us, std::move(future));
}

Buffer RenderController::makeBuffer(Buffer::Type type, Buffer::Usage usage, sp<Uploader> uploader, UploadStrategy us, sp<Future> future)
{
    DTHREAD_CHECK(THREAD_ID_CORE);
    Buffer buffer(_render_engine->rendererFactory()->createBuffer(type, usage));
    if(uploader)
        uploadBuffer(buffer, std::move(uploader), us, std::move(future));
    return buffer;
}

Buffer RenderController::makeBuffer(Buffer::Type type, Buffer::Usage usage, sp<Uploader> uploader)
{
    RenderController::UploadStrategy us = uploader ? RenderController::US_ONCE_AND_ON_SURFACE_READY : RenderController::US_ON_SURFACE_READY;
    return makeBuffer(type, usage, std::move(uploader), us);
}

Buffer RenderController::makeVertexBuffer(Buffer::Usage usage, sp<Uploader> input)
{
    return makeBuffer(Buffer::TYPE_VERTEX, usage, std::move(input));
}

Buffer RenderController::makeIndexBuffer(Buffer::Usage usage, sp<Uploader> input)
{
    return makeBuffer(Buffer::TYPE_INDEX, usage, input);
}

sp<Framebuffer> RenderController::makeFramebuffer(sp<Renderer> renderer, std::vector<sp<Texture>> colorAttachments, sp<Texture> depthStencilAttachments, int32_t clearMask)
{
    const sp<Framebuffer> framebuffer = renderEngine()->rendererFactory()->createFramebuffer(std::move(renderer), std::move(colorAttachments), std::move(depthStencilAttachments), clearMask);
    upload(framebuffer->delegate(), RenderController::US_ONCE_AND_ON_SURFACE_READY, nullptr, nullptr, UPLOAD_PRIORITY_LOW);
    return framebuffer;
}

void RenderController::addPreComposeUpdatable(sp<Updatable> updatable, sp<Boolean> canceled)
{
    DASSERT(updatable && canceled);
    _on_pre_compose_updatable.emplace_back(std::move(updatable), std::move(canceled));
}

void RenderController::addPreComposeRunnable(sp<Runnable> task, sp<Boolean> canceled)
{
    DASSERT(task && canceled);
    _on_pre_compose_runnable.emplace_back(std::move(task), std::move(canceled));
}

void RenderController::addPreRenderRequest(sp<Runnable> task, sp<Boolean> canceled)
{
    ASSERT(task && canceled);
    UpdatableSynchronized<bool> var(std::move(canceled));
    _on_pre_render_runnable.emplace_back(std::move(task), var.synchronized());
    _on_pre_render_sync.push_back(std::move(var));
}

void RenderController::onPreCompose(uint64_t timestamp)
{
    DPROFILER_TRACE("RendererPreUpdate");

    _defered_instances.clear();

    DPROFILER_LOG("Updatables", _on_pre_compose_updatable.items().size());
    for(const sp<Updatable>& i : _on_pre_compose_updatable.update(timestamp))
        i->update(timestamp);

    DPROFILER_LOG("Runnables", _on_pre_compose_runnable.items().size());
    for(const sp<Runnable>& runnable : _on_pre_compose_runnable.update(timestamp))
        runnable->run();

    for(auto iter = _on_pre_render_sync.begin(); iter != _on_pre_render_sync.end(); )
    {
        UpdatableSynchronized<bool>& i = *iter;
        i.update(timestamp);
        if(i.synchronized()->val())
            iter = _on_pre_render_sync.erase(iter);
        else
            ++iter;
    }
}

void RenderController::deferUnref(Box box)
{
    _defered_instances.push_back(std::move(box));
}

sp<SharedIndices> RenderController::getSharedIndices(RenderController::SharedIndicesName name)
{
    DTHREAD_CHECK(THREAD_ID_CORE);
    switch(name) {
    case RenderController::SHARED_INDICES_QUAD:
        return getSharedIndices(RenderUtil::makeUnitQuadModel(), false);
    case RenderController::SHARED_INDICES_NINE_PATCH:
        return getSharedIndices(RenderUtil::makeUnitNinePatchTriangleStripsModel(), true);
    case RenderController::SHARED_INDICES_POINT:
        return getSharedIndices(RenderUtil::makeUnitPointModel(), false);
    default:
        break;
    }
    DFATAL("Unsupported SharedBuffer: %d", name);
    return nullptr;
}

sp<SharedIndices> RenderController::getSharedIndices(const Model& model, bool degenerate)
{
    DTHREAD_CHECK(THREAD_ID_CORE);
    const sp<Uploader>& indicesUploader = model.indices();

    WritableIndice writer(indicesUploader->size() / sizeof(element_index_t));
    indicesUploader->upload(writer);

    uint32_t hash = writer._hash;
    std::vector<element_index_t> indices = std::move(writer._indices);
    const auto iter = _shared_indices.find(hash);
    if(iter != _shared_indices.end())
        return iter->second;

    size_t modelVertexCount = model.vertexCount();

    sp<SharedIndices> sharedBuffer = sp<SharedIndices>::make(makeIndexBuffer(Buffer::USAGE_DYNAMIC), indices, modelVertexCount, degenerate);
    _shared_indices.insert(std::make_pair(hash, sharedBuffer));
    return sharedBuffer;
}

GraphicsBufferAllocator& RenderController::gba()
{
    return _gba;
}

uint64_t RenderController::updateTick()
{
    _tick = _clock->val();
    return _tick;
}

uint64_t RenderController::tick() const
{
    return _tick;
}

RenderController::RenderResource::RenderResource(sp<Resource> resource, sp<Future> future)
    : _resource(std::move(resource)), _future(std::move(future))
{
}

bool RenderController::RenderResource::isExpired() const
{
    return _resource.unique();
}

bool RenderController::RenderResource::isCancelled() const
{
    return _future ? _future->isCancelled() : false;
}

void RenderController::RenderResource::upload(GraphicsContext& graphicsContext) const
{
    _resource->upload(graphicsContext);
}

void RenderController::RenderResource::recycle(GraphicsContext& graphicsContext) const
{
    _resource->recycle()(graphicsContext);
}

uint64_t RenderController::RenderResource::id() const
{
    return _resource->id();
}

RenderController::UploadingRenderResource::UploadingRenderResource(RenderResource resource, UploadStrategy strategy, UploadPriority priority)
    : _resource(std::move(resource)), _strategy(strategy), _priority(priority)
{
}

void RenderController::RenderResourceList::append(UploadPriority priority, RenderResource ur)
{
    _resources[priority].push_back(std::move(ur));
}

void RenderController::RenderResourceList::foreach(GraphicsContext& graphicsContext, bool recycle, bool upload)
{
    for(size_t i = 0; i < UPLOAD_PRIORITY_COUNT; ++i)
        for(auto iter = _resources[i].begin(); iter != _resources[i].end(); )
        {
            const RenderResource& resource = *iter;
            if(resource.isExpired() || resource.isCancelled())
                iter = _resources[i].erase(iter);
            else
            {
                if(recycle)
                    resource.recycle(graphicsContext);
                if(upload)
                    resource.upload(graphicsContext);
                ++iter;
            }
        }
}

template<> void Enums<RenderController::UploadStrategy>::initialize(std::map<String, RenderController::UploadStrategy>& enums)
{
    enums["reload"] = RenderController::US_RELOAD;
    enums["once"] = RenderController::US_ONCE;
    enums["on_surface_ready"] = RenderController::US_ON_SURFACE_READY;
    enums["on_change"] = RenderController::US_ON_CHANGE;
    enums["on_every_frame"] = RenderController::US_ON_EVERY_FRAME;
}

}
