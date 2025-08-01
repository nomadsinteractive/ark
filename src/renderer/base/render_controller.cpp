#include "renderer/base/render_controller.h"

#include "core/base/future.h"
#include "core/inf/runnable.h"
#include "core/inf/writable.h"
#include "core/impl/uploader/uploader_snapshot.h"

#include "graphics/base/bitmap.h"
#include "graphics/components/size.h"

#include "renderer/base/render_target.h"
#include "renderer/base/graphics_context.h"
#include "renderer/base/model.h"
#include "renderer/base/recycler.h"
#include "renderer/base/render_engine.h"
#include "renderer/impl/render_command_composer/rcc_draw_elements.h"
#include "renderer/impl/render_command_composer/rcc_draw_elements_incremental.h"
#include "renderer/inf/renderer_factory.h"
#include "renderer/util/render_util.h"

#include "platform/platform.h"
#include "renderer/inf/pipeline_factory.h"

namespace ark {

namespace {

class WritableIndice final : public Writable {
public:
    WritableIndice(const size_t size)
        : _hash(0), _indices(size) {
    }

    uint32_t write(const void* buffer, const uint32_t size, const uint32_t offset) override {
        _hash = _hash * 101 + RenderUtil::hash(static_cast<const element_index_t*>(buffer), size / sizeof(element_index_t));
        memcpy(_indices.data() + offset, buffer, size);
        return size;
    }

    uint32_t _hash;
    Vector<element_index_t> _indices;
};

class ResourceUploadBufferSnapshot final : public Resource {
public:
    ResourceUploadBufferSnapshot(sp<Buffer::Delegate> buffer, Uploader& uploader)
        : _buffer(std::move(buffer)), _uploader_snapshot(uploader) {
    }

    uint64_t id() override {
        return _buffer->id();
    }

    void upload(GraphicsContext& graphicsContext) override {
        _buffer->uploadBuffer(graphicsContext, _uploader_snapshot);
    }

    ResourceRecycleFunc recycle() override {
        return _buffer->recycle();
    }

private:
    sp<Buffer::Delegate> _buffer;
    UploaderSnapshot _uploader_snapshot;
};

class BufferUpdatable final : public Updatable {
public:
    BufferUpdatable(RenderController& renderController, sp<Uploader> uploader, sp<Buffer::Delegate> buffer)
        : _render_controller(renderController), _buffer(std::move(buffer)), _uploader(std::move(uploader)) {
    }

    bool update(uint64_t timestamp) override {
        const bool dirty = _uploader->update(timestamp) || _buffer->id() == 0;
        if(dirty)
            _render_controller.upload(sp<Resource>::make<ResourceUploadBufferSnapshot>(_buffer, _uploader), enums::UPLOAD_STRATEGY_ONCE);
        return dirty;
    }

private:
    RenderController& _render_controller;
    sp<Buffer::Delegate> _buffer;
    sp<Uploader> _uploader;
};

class UploaderConcat final : public Uploader {
public:
    UploaderConcat(size_t primitiveCount, size_t vertexCount, Vector<element_index_t> indices)
        : Uploader(primitiveCount * indices.size() * sizeof(element_index_t)), _primitive_count(primitiveCount), _vertex_count(vertexCount), _indices(std::move(indices)) {
    }

    void upload(Writable& uploader) override {
        const size_t length = _indices.size();
        const size_t size = length * sizeof(element_index_t);
        size_t offset = 0;
        Vector<element_index_t> indices(_indices);
        element_index_t* buf = indices.data();

        for(size_t i = 0; i < _primitive_count; ++i, offset += size)
        {
            if(i != 0)
                for(size_t j = 0; j < length; ++j)
                    buf[j] += static_cast<element_index_t>(_vertex_count);
            uploader.write(buf, static_cast<uint32_t>(size), static_cast<uint32_t>(offset));
        }
    }

    bool update(uint64_t /*timestamp*/) override {
        return false;
    }

private:
    size_t _primitive_count;
    size_t _vertex_count;
    Vector<element_index_t> _indices;

};

class UploaderDegenerate final : public Uploader {
public:
    UploaderDegenerate(const size_t primitiveCount, const size_t vertexCount, Vector<element_index_t> indices)
        : Uploader(((indices.size() + 2) * primitiveCount - 2) *  sizeof(element_index_t)), _primitive_count(primitiveCount), _vertex_count(vertexCount), _indices(std::move(indices)) {
    }

    void upload(Writable& uploader) override {
        const size_t length = _indices.size();
        const size_t size = length * sizeof(element_index_t);
        uint32_t offset = 0;
        Vector<element_index_t> indices(length + 2);
        element_index_t* buf = indices.data();
        memcpy(buf, _indices.data(), size);

        for(size_t i = 0; i < _primitive_count; ++i, offset += (size + 2 * sizeof(element_index_t)))
        {
            if(i == _primitive_count - 1)
                uploader.write(buf, static_cast<uint32_t>(size), offset);
            else
            {
                buf[length] = buf[length - 1];
                buf[length + 1] = static_cast<element_index_t>(buf[0] + _vertex_count);
                uploader.write(buf, static_cast<uint32_t>(size + 2 * sizeof(element_index_t)), offset);
                for(size_t j = 0; j < length; ++j)
                    buf[j] += static_cast<element_index_t>(_vertex_count);
            }
        }
    }

    bool update(uint64_t /*timestamp*/) override {
        return false;
    }

private:
    size_t _primitive_count;
    size_t _vertex_count;
    Vector<element_index_t> _indices;
};

}

RenderController::PrimitiveIndexBuffer::PrimitiveIndexBuffer(Vector<element_index_t> modelIndices, const size_t modelVertexCount, const bool degenerate, const size_t primitiveCount)
    : _model_indices(std::move(modelIndices)), _model_vertex_count(modelVertexCount), _primitive_count(primitiveCount), _degenerate(degenerate)
{
}

size_t RenderController::PrimitiveIndexBuffer::upload(RenderController& renderController)
{
    sp<Uploader> uploader = _degenerate ? sp<Uploader>::make<UploaderDegenerate>(_primitive_count, _model_vertex_count, _model_indices) : sp<Uploader>::make<UploaderConcat>(_primitive_count, _model_vertex_count, _model_indices);
    const size_t uploaderSize = uploader->size();
    _buffer = renderController.makeBuffer({Buffer::USAGE_BIT_INDEX}, std::move(uploader));
    return uploaderSize;
}

Buffer::Snapshot RenderController::PrimitiveIndexBuffer::snapshot(RenderController& renderController, size_t primitiveCountRequired)
{
    constexpr size_t warningLimit = 20000;
    DCHECK_WARN(primitiveCountRequired < warningLimit, "Object count(%d) exceeding warning limit(%d). You can make the limit larger if you know what you're doing", primitiveCountRequired, warningLimit);
    const size_t size = (_degenerate ? (_model_indices.size() + 2) * primitiveCountRequired - 2 : _model_indices.size() * primitiveCountRequired) * sizeof(element_index_t);
    if(_primitive_count < primitiveCountRequired)
    {
        while(_primitive_count < primitiveCountRequired)
            _primitive_count *= 2;

        const size_t uploaderSize = upload(renderController);
        DCHECK(uploaderSize >= size, "Making Uploader failed, primitive-count: %d, uploader-size: %d, required-size: %d", _primitive_count, uploaderSize, size);
    }
    return _buffer.snapshot(size);
}

RenderController::RenderController(const sp<RenderEngine>& renderEngine, const sp<Dictionary<bitmap>>& bitmapLoader, const sp<Dictionary<bitmap>>& bitmapBoundsLoader)
    : _render_engine(renderEngine), _recycler(sp<Recycler>::make()), _bitmap_loader(bitmapLoader), _bitmap_bounds_loader(bitmapBoundsLoader), _gba(*this)
{
}

void RenderController::reset()
{
    DTHREAD_CHECK(THREAD_ID_CORE);
    _shared_primitive_index_buffer.clear();
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
            if(front._strategy == enums::UPLOAD_STRATEGY_RELOAD && front._resource.id() != 0)
                front._resource.recycle(graphicsContext);

            if((front._strategy.has(enums::UPLOAD_STRATEGY_ONCE)) || front._strategy == enums::UPLOAD_STRATEGY_RELOAD)
                front._resource.upload(graphicsContext);

            if(front._strategy.has(enums::UPLOAD_STRATEGY_ON_EVERY_FRAME))
                _on_every_frame.append(front._priority, std::move(front._resource));
            else if(front._strategy.has(enums::UPLOAD_STRATEGY_ON_SURFACE_READY))
                _on_surface_ready.append(front._priority, std::move(front._resource));
        }
    }
}

void RenderController::onDrawFrame(GraphicsContext& graphicsContext)
{
    prepare(graphicsContext, _uploading_resources);
    _on_every_frame.foreach(graphicsContext, false, true);

    if(const uint32_t tick = graphicsContext.tick() % 300; tick == 0)
        _on_surface_ready.foreach(graphicsContext, false, false);
    else if (tick == 150)
        _recycler->doRecycling(graphicsContext);

    for(const sp<Runnable>& runnable : _on_pre_render_runnable.update(0))
        runnable->run();
}

void RenderController::upload(sp<Resource> resource, const enums::UploadStrategy strategy, sp<Updatable> updatable, sp<Future> future, const enums::UploadPriority priority)
{
    if(strategy.has(enums::UPLOAD_STRATEGY_ON_CHANGE))
    {
        CHECK(updatable, "An updatable must be specified using \"on_change\" upload strategy");
        sp<Boolean> discarded = future ? future->isCanceled() : sp<Boolean>::make<BooleanByWeakRef<Resource>>(resource, 1);
        addPreComposeUpdatable(std::move(updatable), std::move(discarded));
    }
    if(strategy != enums::UPLOAD_STRATEGY_ON_CHANGE)
        _uploading_resources.push(UploadingRenderResource(RenderResource(std::move(resource), std::move(future)), strategy, priority));
}

void RenderController::uploadBuffer(const Buffer& buffer, sp<Uploader> uploader, const enums::UploadStrategy strategy, sp<Future> future, enums::UploadPriority priority)
{
    ASSERT(uploader);
    if(!future)
        future = sp<Future>::make(nullptr, sp<Boolean>::make<BooleanByWeakRef<Buffer::Delegate>>(buffer.delegate(), 1));
    buffer.delegate()->setSize(uploader->size());
    Uploader& uploaderInstance = *uploader;
    sp<Updatable> updatable = strategy.has(enums::UPLOAD_STRATEGY_ON_CHANGE) ? sp<Updatable>::make<BufferUpdatable>(*this, std::move(uploader), buffer.delegate()) : sp<Updatable>();
    //TODO: make this mess a bit more cleaner
    if(strategy == enums::UPLOAD_STRATEGY_ON_CHANGE)
        upload(nullptr, strategy, std::move(updatable), std::move(future), priority);
    else
        upload(sp<Resource>::make<ResourceUploadBufferSnapshot>(buffer.delegate(), uploaderInstance), strategy, std::move(updatable), std::move(future), priority);
}

const sp<Recycler>& RenderController::recycler() const
{
    return _recycler;
}

const sp<RenderEngine>& RenderController::renderEngine() const
{
    return _render_engine;
}

sp<Texture> RenderController::createTexture(sp<Size> size, sp<Texture::Parameters> parameters, sp<Texture::Uploader> uploader, const enums::UploadStrategy us, sp<Future> future)
{
    sp<Texture::Delegate> delegate = _render_engine->rendererFactory()->createTexture(size, parameters);
    DCHECK(delegate, "Unsupported TextureType: %d", parameters->_type);
    sp<Texture> texture = sp<Texture>::make(std::move(delegate), std::move(size), std::move(uploader), std::move(parameters));
    upload(texture, us, nullptr, std::move(future));
    return texture;
}

sp<Texture> RenderController::createTexture2d(sp<Bitmap> bitmap, Texture::Format format, const enums::UploadStrategy us, sp<Future> future)
{
    sp<Size> size = sp<Size>::make(static_cast<float>(bitmap->width()), static_cast<float>(bitmap->height()));
    return createTexture(std::move(size), sp<Texture::Parameters>::make(Texture::TYPE_2D, nullptr, format), sp<Texture::UploaderBitmap>::make(std::move(bitmap)), us, std::move(future));
}

Buffer RenderController::makeBuffer(const Buffer::Usage usage, sp<Uploader> uploader, const enums::UploadStrategy us, sp<Future> future)
{
    DTHREAD_CHECK(THREAD_ID_CORE);
    Buffer buffer(_render_engine->rendererFactory()->createBuffer(usage));
    if(uploader)
        uploadBuffer(buffer, std::move(uploader), us, std::move(future));
    return buffer;
}

Buffer RenderController::makeBuffer(const Buffer::Usage usage, sp<Uploader> uploader)
{
    enums::UploadStrategy us = uploader ? enums::UPLOAD_STRATEGY_ONCE_AND_ON_SURFACE_READY : enums::UPLOAD_STRATEGY_ON_SURFACE_READY;
    if(usage.has(Buffer::USAGE_BIT_DYNAMIC) && uploader)
        us = us | enums::UPLOAD_STRATEGY_ON_CHANGE;
    return makeBuffer(usage, std::move(uploader), us);
}

Buffer RenderController::makeVertexBuffer(const Buffer::Usage usage, sp<Uploader> uploader)
{
    return makeBuffer(usage | Buffer::USAGE_BIT_VERTEX, std::move(uploader));
}

Buffer RenderController::makeIndexBuffer(const Buffer::Usage usage, sp<Uploader> uploader)
{
    return makeBuffer(usage | Buffer::USAGE_BIT_INDEX, std::move(uploader));
}

sp<RenderController::PrimitiveIndexBuffer> RenderController::getSharedPrimitiveIndexBuffer(const Model& model, bool degenerate)
{
    DTHREAD_CHECK(THREAD_ID_CORE);
    const sp<Uploader>& indicesUploader = model.indices();

    WritableIndice writer(indicesUploader->size() / sizeof(element_index_t));
    indicesUploader->upload(writer);

    const uint32_t hash = writer._hash;
    Vector<element_index_t> indices = std::move(writer._indices);
    if(const auto iter = _shared_primitive_index_buffer.find(hash); iter != _shared_primitive_index_buffer.end())
        return iter->second;

    sp<PrimitiveIndexBuffer>& pib = _shared_primitive_index_buffer[hash];
    pib = sp<PrimitiveIndexBuffer>::make(indices, model.vertexCount(), degenerate, 128);
    pib->upload(*this);
    return pib;
}

sp<RenderTarget> RenderController::makeRenderTarget(sp<Renderer> renderer, RenderTarget::Configure configure)
{
    sp<RenderTarget> renderTarget = renderEngine()->rendererFactory()->createRenderTarget(std::move(renderer), std::move(configure));
    if(renderTarget->resource())
        upload(renderTarget->resource(), enums::UPLOAD_STRATEGY_ONCE_AND_ON_SURFACE_READY, nullptr, nullptr, enums::UPLOAD_PRIORITY_LOW);
    return renderTarget;
}

sp<DrawingContextComposer> RenderController::makeDrawElementsIncremental(sp<Model> model) const
{
    return _render_engine->rendererFactory()->features()._can_draw_element_incremental ? sp<DrawingContextComposer>::make<RCCDrawElementsIncremental>()
                                                                                       : sp<DrawingContextComposer>::make<RCCDrawElements>(std::move(model));
}

void RenderController::addPreComposeUpdatable(sp<Updatable> updatable, sp<Boolean> canceled)
{
    DASSERT(updatable && canceled);
    _on_pre_compose_updatable.emplace_back(std::move(updatable), std::move(canceled));
}

void RenderController::addPreComposeRunnable(sp<Runnable> task, sp<Boolean> canceled)
{
    ASSERT(task);
    CHECK_WARN(canceled, "Adding an undisposable task to application");
    _on_pre_compose_runnable.emplace_back(std::move(task), std::move(canceled));
}

void RenderController::addPreRenderRequest(sp<Runnable> task, sp<Boolean> canceled)
{
    ASSERT(task && canceled);
    UpdatableSynchronized<bool> var(std::move(canceled));
    _on_pre_render_runnable.emplace_back(std::move(task), var.synchronized());
    _on_pre_render_sync.push_back(std::move(var));
}

void RenderController::onPreCompose(const uint64_t timestamp)
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

GraphicsBufferAllocator& RenderController::gba()
{
    return _gba;
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
    return _future ? _future->isCanceled()->val() : false;
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

RenderController::UploadingRenderResource::UploadingRenderResource(RenderResource resource, const enums::UploadStrategy strategy, const enums::UploadPriority priority)
    : _resource(std::move(resource)), _strategy(strategy), _priority(priority)
{
}

void RenderController::RenderResourceList::append(enums::UploadPriority priority, RenderResource ur)
{
    _resources[priority].push_back(std::move(ur));
}

void RenderController::RenderResourceList::foreach(GraphicsContext& graphicsContext, bool recycle, bool upload)
{
    for(size_t i = 0; i < enums::UPLOAD_PRIORITY_COUNT; ++i)
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

}
