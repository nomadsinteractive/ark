#include "renderer/base/render_controller.h"

#include "core/base/manifest.h"
#include "core/base/future.h"
#include "core/inf/runnable.h"
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
#include "renderer/impl/uploader/uploader_recoder.h"
#include "renderer/util/render_util.h"

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

class UploadingBufferResource : public Resource {
public:
    UploadingBufferResource(sp<Buffer::Delegate> buffer, sp<Uploader> uploader)
        : _buffer(std::move(buffer)), _uploader(std::move(uploader)) {
    }

    virtual uint64_t id() override {
        return _buffer->id();
    }

    virtual void upload(GraphicsContext& graphicsContext) override {
        _buffer->uploadBuffer(graphicsContext, _uploader);
    }

    virtual ResourceRecycleFunc recycle() override {
        return _buffer->recycle();
    }

private:
    sp<Buffer::Delegate> _buffer;
    sp<Uploader> _uploader;
};

class BufferUpdatable : public Updatable {
public:
    BufferUpdatable(RenderController& renderController, sp<Updatable> updatable, sp<Uploader> uploader, sp<Buffer::Delegate> buffer)
        : _render_controller(renderController), _updatable(std::move(updatable)), _uploader(std::move(uploader)), _buffer(std::move(buffer)) {
    }

    virtual bool update(uint64_t timestamp) override {
        bool dirty = _updatable->update(timestamp) || _buffer->id() == 0;
        if(dirty)
            _render_controller.upload(sp<UploadingBufferResource>::make(_buffer, sp<UploaderRecorder>::make(_uploader)), RenderController::US_ONCE);
        return dirty;
    }

private:
    RenderController& _render_controller;
    sp<Updatable> _updatable;
    sp<Uploader> _uploader;
    sp<Buffer::Delegate> _buffer;
};

}

RenderController::RenderController(const sp<RenderEngine>& renderEngine, const sp<Recycler>& recycler, const sp<Dictionary<bitmap>>& bitmapLoader, const sp<Dictionary<bitmap>>& bitmapBoundsLoader)
    : _render_engine(renderEngine), _recycler(recycler), _bitmap_loader(bitmapLoader), _bitmap_bounds_loader(bitmapBoundsLoader), _clock(Platform::getSteadyClock())
{
}

void RenderController::reset()
{
    DTHREAD_CHECK(THREAD_ID_CORE);
    _shared_indices.clear();
}

void RenderController::onSurfaceReady(GraphicsContext& graphicsContext)
{
    doRecycling(graphicsContext);
    doSurfaceReady(graphicsContext);
}

void RenderController::prepare(GraphicsContext& graphicsContext, LFQueue<UploadingResource>& items)
{
    UploadingResource front;
    while(items.pop(front)) {
        if(!front._resource.isCancelled())
        {
            if(front._strategy == RenderController::US_RELOAD && front._resource.id() != 0)
                front._resource.recycle(graphicsContext);

            front._resource.upload(graphicsContext);
            if(front._strategy & RenderController::US_ON_SURFACE_READY)
                _on_surface_ready_items.insert(std::move(front._resource));
        }
    }
}

void RenderController::onDrawFrame(GraphicsContext& graphicsContext)
{
    prepare(graphicsContext, _uploading_resources);

    uint32_t tick = graphicsContext.tick();
    if(tick == 0)
        doRecycling(graphicsContext);
    else if (tick == 150)
        _recycler->doRecycling(graphicsContext);
}

void RenderController::upload(sp<Resource> resource, RenderController::UploadStrategy strategy, sp<Future> future, RenderController::UploadPriority priority)
{
    switch(strategy & 3)
    {
    case RenderController::US_ONCE_AND_ON_SURFACE_READY:
    case RenderController::US_ONCE:
    case RenderController::US_RELOAD:
        _uploading_resources.push(UploadingResource(PreUploadingResource(std::move(resource), std::move(future), priority), strategy));
        break;
    case RenderController::US_ON_SURFACE_READY:
        _on_surface_ready_items.insert(PreUploadingResource(std::move(resource), std::move(future), priority));
        break;
    }
}

void RenderController::uploadBuffer(Buffer& buffer, sp<Uploader> uploader, RenderController::UploadStrategy strategy, sp<Future> future, RenderController::UploadPriority priority)
{
    if(strategy & RenderController::US_ON_CHANGED)
    {
        sp<Boolean> disposed = future ? future->cancelled() : sp<Boolean>::make<BooleanByWeakRef<Buffer::Delegate>>(buffer.delegate(), 2);
        addPreRenderUpdateRequest(sp<BufferUpdatable>::make(*this, uploader->updatable(), uploader, buffer.delegate()), disposed);
    }
    else if(uploader)
    {
        buffer._resource = sp<UploadingBufferResource>::make(buffer._delegate, std::move(uploader));
        upload(buffer._resource, strategy, std::move(future), priority);
    }
}

const sp<Recycler>& RenderController::recycler() const
{
    return _recycler;
}

void RenderController::doRecycling(GraphicsContext& graphicsContext)
{
    for(auto iter = _on_surface_ready_items.begin(); iter != _on_surface_ready_items.end(); )
    {
        const PreUploadingResource& resource = *iter;
        if(resource.isExpired() || resource.isCancelled())
        {
            resource.recycle(graphicsContext);
            iter = _on_surface_ready_items.erase(iter);
        }
        else
            ++iter;
    }
}

void RenderController::doSurfaceReady(GraphicsContext& graphicsContext) const
{
    for(const PreUploadingResource& resource : _on_surface_ready_items)
        resource.recycle(graphicsContext);

    uploadSurfaceReadyItems(graphicsContext, UPLOAD_PRIORITY_HIGH);
    uploadSurfaceReadyItems(graphicsContext, UPLOAD_PRIORITY_NORMAL);
    uploadSurfaceReadyItems(graphicsContext, UPLOAD_PRIORITY_LOW);
}

void RenderController::uploadSurfaceReadyItems(GraphicsContext& graphicsContext, UploadPriority up) const
{
    for(const PreUploadingResource& resource : _on_surface_ready_items)
        if(resource.uploadPriority() == up)
        {
            DWARN(resource.id() == 0, "Resource[%d] has been uploaded, please check your UploadPriority", resource.id());
            resource.upload(graphicsContext);
        }
}

const sp<RenderEngine>& RenderController::renderEngine() const
{
    return _render_engine;
}

sp<Camera> RenderController::createCamera() const
{
    Ark::RendererCoordinateSystem cs = _render_engine->context()->coordinateSystem();
    return sp<Camera>::make(cs, _render_engine->rendererFactory()->createCamera(cs));
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
    upload(texture, us, std::move(future));
    return texture;
}

sp<Texture> RenderController::createTexture2D(sp<Size> size, sp<Texture::Uploader> uploader, RenderController::UploadStrategy us, sp<Future> future)
{
    return createTexture(std::move(size), sp<Texture::Parameters>::make(Texture::TYPE_2D, 0), std::move(uploader), us, std::move(future));
}

Buffer RenderController::makeVertexBuffer(Buffer::Usage usage, const sp<Uploader>& uploader)
{
    return makeBuffer(Buffer::TYPE_VERTEX, usage, uploader);
}

Buffer RenderController::makeIndexBuffer(Buffer::Usage usage, const sp<Uploader>& uploader)
{
    return makeBuffer(Buffer::TYPE_INDEX, usage, uploader);
}

sp<Framebuffer> RenderController::makeFramebuffer(sp<Renderer> renderer, std::vector<sp<Texture>> colorAttachments, sp<Texture> depthStencilAttachments, int32_t clearMask)
{
    const sp<Framebuffer> framebuffer = renderEngine()->rendererFactory()->createFramebuffer(std::move(renderer), std::move(colorAttachments), std::move(depthStencilAttachments), clearMask);
    upload(framebuffer->delegate(), RenderController::US_ONCE_AND_ON_SURFACE_READY, nullptr, UPLOAD_PRIORITY_LOW);
    return framebuffer;
}

Buffer RenderController::makeBuffer(Buffer::Type type, Buffer::Usage usage, const sp<Uploader>& uploader)
{
    DTHREAD_CHECK(THREAD_ID_CORE);
    Buffer buffer(_render_engine->rendererFactory()->createBuffer(type, usage));
    uploadBuffer(buffer, uploader, uploader ? RenderController::US_ONCE_AND_ON_SURFACE_READY : RenderController::US_ON_SURFACE_READY);
    return buffer;
}

void RenderController::addPreRenderUpdateRequest(const sp<Updatable>& updatable, const sp<Boolean>& disposed)
{
    _on_pre_updatable.push_back(updatable, disposed ? disposed : sp<Boolean>::make<BooleanByWeakRef<Updatable>>(updatable, 1));
}

void RenderController::addPreRenderRunRequest(const sp<Runnable>& task, const sp<Boolean>& disposed)
{
    if(disposed)
        _on_pre_update_request.push_back(task, disposed);
    else
    {
        const sp<Disposed> e = task.as<Disposed>();
        DCHECK(e, "Adding an undisposable running task, it's that what you REALLY want?");
        _on_pre_update_request.push_back(task, e);
    }
}

void RenderController::preRequestUpdate(uint64_t timestamp)
{
    DPROFILER_TRACE("RendererPreUpdate");

    _defered_instances.clear();

    DPROFILER_LOG("Updatables", _on_pre_updatable.items().size());
    for(const sp<Updatable>& i : _on_pre_updatable.update(timestamp))
        i->update(timestamp);

    DPROFILER_LOG("Runnables", _on_pre_update_request.items().size());
    for(const sp<Runnable>& runnable : _on_pre_update_request.update(timestamp))
        runnable->run();
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

uint64_t RenderController::updateTick()
{
    _tick = _clock->val();
    return _tick;
}

uint64_t RenderController::tick() const
{
    return _tick;
}

RenderController::PreUploadingResource::PreUploadingResource(sp<Resource> resource, sp<Future> future, UploadPriority uploadPriority)
    : _resource(std::move(resource)), _future(std::move(future)), _upload_priority(uploadPriority)
{
}

bool RenderController::PreUploadingResource::isExpired() const
{
    return _resource.unique();
}

bool RenderController::PreUploadingResource::isCancelled() const
{
    return _future ? _future->isCancelled() : false;
}

void RenderController::PreUploadingResource::upload(GraphicsContext& graphicsContext) const
{
    _resource->upload(graphicsContext);
}

void RenderController::PreUploadingResource::recycle(GraphicsContext& graphicsContext) const
{
    _resource->recycle()(graphicsContext);
}

uint64_t RenderController::PreUploadingResource::id() const
{
    return _resource->id();
}

RenderController::UploadPriority RenderController::PreUploadingResource::uploadPriority() const
{
    return _upload_priority;
}

bool RenderController::PreUploadingResource::operator <(const RenderController::PreUploadingResource& other) const
{
    return _resource < other._resource;
}

RenderController::UploadingResource::UploadingResource(PreUploadingResource resource, RenderController::UploadStrategy strategy)
    : _resource(std::move(resource)), _strategy(strategy)
{
}

bool RenderController::UploadingResource::operator <(const RenderController::UploadingResource& other) const
{
    return _resource < other._resource;
}

}
