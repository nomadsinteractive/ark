#include "renderer/base/render_controller.h"

#include "core/base/manifest.h"
#include "core/base/future.h"
#include "core/inf/runnable.h"
#include "core/util/log.h"

#include "graphics/base/bitmap.h"
#include "graphics/base/size.h"

#include "renderer/base/framebuffer.h"
#include "renderer/base/graphics_context.h"
#include "renderer/base/model.h"
#include "renderer/base/recycler.h"
#include "renderer/base/render_engine.h"
#include "renderer/base/render_engine_context.h"
#include "renderer/base/shared_buffer.h"
#include "renderer/inf/renderer_factory.h"
#include "renderer/inf/snippet_factory.h"
#include "renderer/inf/vertices.h"
#include "renderer/util/render_util.h"

namespace ark {

namespace {

class WritableIndiceHash : public Writable {
public:
    WritableIndiceHash()
        : _hash(0) {
    }

    virtual uint32_t write(const void* buffer, uint32_t size, uint32_t offset) override {
        _hash += RenderUtil::hash(reinterpret_cast<const element_index_t*>(buffer), size / sizeof(element_index_t));
        return size;
    }

    element_index_t _hash;
};

}

RenderController::RenderController(const sp<RenderEngine>& renderEngine, const sp<Recycler>& recycler, const sp<Dictionary<bitmap>>& bitmapLoader, const sp<Dictionary<bitmap>>& bitmapBoundsLoader)
    : _render_engine(renderEngine), _recycler(recycler), _bitmap_loader(bitmapLoader), _bitmap_bounds_loader(bitmapBoundsLoader), _clock(Platform::getSteadyClock())
{
}

void RenderController::reset()
{
    DTHREAD_CHECK(THREAD_ID_CORE);
    _shared_buffers.clear();
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
        if(!front._resource.isCancelled() && (!front._resource.isExpired() || front._strategy == RenderController::US_RELOAD))
        {
            if(front._strategy == RenderController::US_RELOAD && front._resource.resource()->id() != 0)
                front._resource.recycle(graphicsContext);

            front._resource.upload(graphicsContext);
            if(front._strategy == RenderController::US_ONCE_AND_ON_SURFACE_READY)
                _on_surface_ready_items.insert(std::move(front._resource));
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

sp<Future> RenderController::upload(sp<Resource> resource, sp<Uploader> uploader, RenderController::UploadStrategy strategy, RenderController::UploadPriority priority)
{
    switch(strategy & 3)
    {
    case RenderController::US_ONCE_AND_ON_SURFACE_READY:
    case RenderController::US_ONCE:
    case RenderController::US_RELOAD:
        {
            sp<Future> future = sp<Future>::make();
            _preparing_items.push(PreparingResource(RenderResource(std::move(resource), std::move(uploader), future, priority), strategy));
            return future;
        }
    case RenderController::US_ON_SURFACE_READY:
        {
            sp<Future> future = sp<Future>::make();
            _on_surface_ready_items.insert(RenderResource(std::move(resource), std::move(uploader), future, priority));
            return future;
        }
    }
    return nullptr;
}

sp<Future> RenderController::uploadBuffer(const Buffer& buffer, const sp<Uploader>& uploader, RenderController::UploadStrategy strategy, RenderController::UploadPriority priority)
{
    return upload(buffer._delegate, uploader, strategy, priority);
}

const sp<Recycler>& RenderController::recycler() const
{
    return _recycler;
}

void RenderController::doRecycling(GraphicsContext& graphicsContext)
{
    for(auto iter = _on_surface_ready_items.begin(); iter != _on_surface_ready_items.end(); )
    {
        const RenderResource& resource = *iter;
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
    for(const RenderResource& resource : _on_surface_ready_items)
        resource.recycle(graphicsContext);

    uploadSurfaceReadyItems(graphicsContext, UPLOAD_PRIORITY_HIGH);
    uploadSurfaceReadyItems(graphicsContext, UPLOAD_PRIORITY_NORMAL);
    uploadSurfaceReadyItems(graphicsContext, UPLOAD_PRIORITY_LOW);
}

void RenderController::uploadSurfaceReadyItems(GraphicsContext& graphicsContext, UploadPriority up) const
{
    for(const RenderResource& resource : _on_surface_ready_items)
        if(resource.uploadPriority() == up)
        {
            DWARN(resource.id() == 0, "Resource[%d] has been uploaded, please check your UploadPriority", resource.id());
            resource.upload(graphicsContext);
        }
}

element_index_t RenderController::getIndicesHash(Uploader& indices) const
{
    WritableIndiceHash writer;
    indices.upload(writer);
    return writer._hash;
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

sp<Texture> RenderController::createTexture(sp<Size> size, sp<Texture::Parameters> parameters, sp<Texture::Uploader> uploader, RenderController::UploadStrategy us)
{
    sp<Texture::Delegate> delegate = _render_engine->rendererFactory()->createTexture(size, parameters);
    DCHECK(delegate, "Unsupported TextureType: %d", parameters->_type);
    const sp<Texture> texture = sp<Texture>::make(std::move(delegate), std::move(size), std::move(uploader), std::move(parameters));
    if(us != RenderController::US_MANUAL)
        upload(texture, nullptr, us);
    return texture;
}

sp<Texture> RenderController::createTexture2D(sp<Size> size, sp<Texture::Uploader> uploader, RenderController::UploadStrategy us)
{
    return createTexture(std::move(size), sp<Texture::Parameters>::make(Texture::TYPE_2D, 0), std::move(uploader), us);
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
    upload(framebuffer->delegate(), nullptr, RenderController::US_ONCE_AND_ON_SURFACE_READY, UPLOAD_PRIORITY_LOW);
    return framebuffer;
}

Buffer RenderController::makeBuffer(Buffer::Type type, Buffer::Usage usage, const sp<Uploader>& uploader)
{
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

void RenderController::preUpdate(uint64_t timestamp)
{
#ifdef ARK_FLAG_DEBUG
    static int i = 0;
    if(i ++ == 600)
    {
        LOGD("_on_pre_updatable: %d", _on_pre_updatable.items().size());
        LOGD("_on_pre_update_request: %d", _on_pre_update_request.items().size());
        i = 0;
    }
#endif
    _defered_instances.clear();

    for(const sp<Updatable>& i : _on_pre_updatable.update(timestamp))
        i->update(timestamp);

    for(const sp<Runnable>& runnable : _on_pre_update_request.update(timestamp))
        runnable->run();
}

void RenderController::deferUnref(Box box)
{
    _defered_instances.push_back(std::move(box));
}

sp<SharedBuffer> RenderController::getNamedBuffer(SharedBuffer::Name name)
{
    DTHREAD_CHECK(THREAD_ID_CORE);
    switch(name) {
    case SharedBuffer::NAME_QUADS:
        return getSharedBuffer(ModelLoader::RENDER_MODE_TRIANGLES, RenderUtil::makeUnitQuadModel());
    case SharedBuffer::NAME_NINE_PATCH:
        return getSharedBuffer(ModelLoader::RENDER_MODE_TRIANGLE_STRIP, RenderUtil::makeUnitNinePatchTriangleStripsModel());
    case SharedBuffer::NAME_POINTS:
        return getSharedBuffer(ModelLoader::RENDER_MODE_POINTS, RenderUtil::makeUnitPointModel());
    default:
        break;
    }
    DFATAL("Unsupported SharedBuffer: %d", name);
    return nullptr;
}

sp<SharedBuffer> RenderController::getSharedBuffer(ModelLoader::RenderMode renderMode, const Model& model)
{
    const sp<Uploader>& indices = model.indices();
    element_index_t hash = getIndicesHash(indices);
    const auto iter = _shared_buffers.find(hash);
    if(iter != _shared_buffers.end())
        return iter->second;

    bool degenerate = renderMode == ModelLoader::RENDER_MODE_TRIANGLE_STRIP;
    size_t modelIndexCount = model.indexCount();
    size_t modelVertexCount = model.vertexCount();

    sp<SharedBuffer> sharedBuffer;
    if(degenerate)
        sharedBuffer = sp<SharedBuffer>::make(makeIndexBuffer(Buffer::USAGE_STATIC),
                                             [indices, modelVertexCount](size_t primitiveCount)->sp<Uploader> { return sp<SharedBuffer::Degenerate>::make(primitiveCount, modelVertexCount, indices); },
                                             [modelIndexCount](size_t primitiveCount)->size_t { return ((modelIndexCount + 2) * primitiveCount - 2) * sizeof(element_index_t); });
    else
        sharedBuffer = sp<SharedBuffer>::make(makeIndexBuffer(Buffer::USAGE_STATIC),
                                             [indices, modelVertexCount](size_t primitiveCount)->sp<Uploader> { return sp<SharedBuffer::Concat>::make(primitiveCount, modelVertexCount, indices); },
                                             [modelIndexCount](size_t primitiveCount)->size_t { return modelIndexCount * primitiveCount * sizeof(element_index_t); });
    _shared_buffers.insert(std::make_pair(hash, sharedBuffer));
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

RenderController::RenderResource::RenderResource(sp<Resource> resource, sp<Uploader> uploader, sp<Future> future, UploadPriority uploadPriority)
    : _resource(std::move(resource)), _uploader(std::move(uploader)), _future(std::move(future)), _upload_priority(uploadPriority)
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

bool RenderController::RenderResource::isCancelled() const
{
    return _future->isCancelled();
}

void RenderController::RenderResource::upload(GraphicsContext& graphicsContext) const
{
    _resource->upload(graphicsContext, _uploader);
}

void RenderController::RenderResource::recycle(GraphicsContext& graphicsContext) const
{
    _resource->recycle()(graphicsContext);
}

uint64_t RenderController::RenderResource::id() const
{
    return _resource->id();
}

RenderController::UploadPriority RenderController::RenderResource::uploadPriority() const
{
    return _upload_priority;
}

bool RenderController::RenderResource::operator <(const RenderController::RenderResource& other) const
{
    return _resource < other._resource;
}

RenderController::PreparingResource::PreparingResource(RenderResource resource, RenderController::UploadStrategy strategy)
    : _resource(std::move(resource)), _strategy(strategy)
{
}

bool RenderController::PreparingResource::operator <(const RenderController::PreparingResource& other) const
{
    return _resource < other._resource;
}

}
