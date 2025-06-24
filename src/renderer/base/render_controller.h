#pragma once

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/base/bit_set.h"
#include "core/collection/list.h"
#include "core/concurrent/lf_queue.h"
#include "core/inf/runnable.h"
#include "core/inf/variable.h"
#include "core/impl/boolean/boolean_by_weak_ref.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "renderer/forwarding.h"
#include "renderer/base/buffer.h"
#include "renderer/base/graphics_buffer_allocator.h"
#include "renderer/base/render_target.h"
#include "renderer/base/texture.h"
#include "renderer/inf/model_loader.h"


namespace ark {

class ARK_API RenderController {
public:
    class ARK_API PrimitiveIndexBuffer {
    public:
        PrimitiveIndexBuffer(Vector<element_index_t> modelIndices, size_t modelVertexCount, bool degenerate, size_t primitiveCount);

        size_t upload(RenderController& renderController);
        Buffer::Snapshot snapshot(RenderController& renderController, size_t primitiveCountRequired);

    private:
        Vector<element_index_t> _model_indices;
        size_t _model_vertex_count;
        size_t _primitive_count;

        Buffer _buffer;
        bool _degenerate;
    };

private:
    template<typename T> class UpdatableSynchronized final : public Updatable {
    public:
        UpdatableSynchronized(sp<Variable<T>> delegate)
            : _delegate(std::move(delegate)), _synchronized(sp<typename Variable<T>::Impl>::make(_delegate->val())) {
        }

        const sp<typename Variable<T>::Impl>& synchronized() const {
            return _synchronized;
        }

        bool update(uint64_t timestamp) override {
            if(_delegate->update(timestamp)) {
                _synchronized->set(_delegate->val());
                return true;
            }
            return false;
        }

    private:
        sp<Variable<T>> _delegate;
        sp<typename Variable<T>::Impl> _synchronized;

    };

public:
    RenderController(const sp<RenderEngine>& renderEngine, const sp<Dictionary<bitmap>>& bitmapLoader, const sp<Dictionary<bitmap>>& bitmapBoundsLoader);

    void reset();

    const sp<Recycler>& recycler() const;

    void onSurfaceReady(GraphicsContext& graphicsContext);
    void onSurfaceDestroy(GraphicsContext& graphicsContext);

    void onDrawFrame(GraphicsContext& graphicsContext);

    void upload(sp<Resource> resource, enums::UploadStrategy strategy, sp<Updatable> updatable = nullptr, sp<Future> future = nullptr, enums::UploadPriority priority = enums::UPLOAD_PRIORITY_NORMAL);

//  [[script::bindings::auto]]
    void uploadBuffer(const Buffer& buffer, sp<Uploader> input, enums::UploadStrategy strategy, sp<Future> future = nullptr, enums::UploadPriority priority = enums::UPLOAD_PRIORITY_NORMAL);

    const sp<RenderEngine>& renderEngine() const;

    sp<Texture> createTexture(sp<Size> size, sp<Texture::Parameters> parameters, sp<Texture::Uploader> uploader, enums::UploadStrategy us = enums::UPLOAD_STRATEGY_ONCE_AND_ON_SURFACE_READY, sp<Future> future = nullptr);
//  [[script::bindings::auto]]
    sp<Texture> createTexture2d(sp<Bitmap> bitmap, Texture::Format format = Texture::FORMAT_AUTO, enums::UploadStrategy us = enums::UPLOAD_STRATEGY_ONCE_AND_ON_SURFACE_READY, sp<Future> future = nullptr);

//  [[script::bindings::auto]]
    Buffer makeBuffer(Buffer::Usage usage, sp<Uploader> uploader, enums::UploadStrategy us, sp<Future> future = nullptr);
    Buffer makeBuffer(Buffer::Usage usage, sp<Uploader> uploader);
//  [[script::bindings::auto]]
    Buffer makeVertexBuffer(Buffer::Usage usage = Buffer::USAGE_BIT_DYNAMIC, sp<Uploader> uploader = nullptr);
//  [[script::bindings::auto]]
    Buffer makeIndexBuffer(Buffer::Usage usage = Buffer::USAGE_BIT_DYNAMIC, sp<Uploader> uploader = nullptr);

    sp<PrimitiveIndexBuffer> getSharedPrimitiveIndexBuffer(const Model& model, bool degenerate);

    sp<RenderTarget> makeRenderTarget(sp<Renderer> renderer, RenderTarget::Configure configure);

    sp<DrawingContextComposer> makeDrawElementsIncremental(sp<Model> model) const;

    template<typename T> sp<Variable<T>> synchronize(sp<Variable<T>> delegate, sp<Boolean> canceled) {
        const sp<UpdatableSynchronized<T>> s = sp<UpdatableSynchronized<T>>::make(std::move(delegate));
        const sp<Variable<T>>& var = s->synchronized();
        _on_pre_compose_updatable.emplace_back(s, canceled ? std::move(canceled) : sp<Boolean>::make<BooleanByWeakRef<Variable<T>>>(var, 1));
        return var;
    }

    void addPreComposeUpdatable(sp<Updatable> updatable, sp<Boolean> canceled);
    void addPreComposeRunnable(sp<Runnable> task, sp<Boolean> canceled);

    void addPreRenderRequest(sp<Runnable> task, sp<Boolean> canceled);

    void onPreCompose(uint64_t timestamp);
    void deferUnref(Box box);

    GraphicsBufferAllocator& gba();

private:
    class RenderResource {
    public:
        RenderResource() = default;
        RenderResource(sp<Resource> resource, sp<Future> future);
        DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(RenderResource);

        bool isExpired() const;
        bool isCancelled() const;

        void upload(GraphicsContext& graphicsContext) const;
        void recycle(GraphicsContext& graphicsContext) const;

        uint64_t id() const;

    private:
        sp<Resource> _resource;
        sp<Future> _future;
    };

    struct UploadingRenderResource {
        UploadingRenderResource() = default;
        UploadingRenderResource(RenderResource resource, enums::UploadStrategy strategy, enums::UploadPriority priority);
        DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(UploadingRenderResource);

        RenderResource _resource;
        enums::UploadStrategy _strategy;
        enums::UploadPriority _priority;
    };

    class RenderResourceList {
    public:
        void append(enums::UploadPriority priority, RenderResource ur);

        void foreach(GraphicsContext& graphicsContext, bool recycle, bool upload);

    private:
        Vector<RenderResource> _resources[enums::UPLOAD_PRIORITY_COUNT];
    };

private:
    void prepare(GraphicsContext& graphicsContext, LFQueue<UploadingRenderResource>& items);

private:
    sp<RenderEngine> _render_engine;
    sp<Recycler> _recycler;
    sp<Dictionary<bitmap>> _bitmap_loader;
    sp<Dictionary<bitmap>> _bitmap_bounds_loader;

    LFQueue<UploadingRenderResource> _uploading_resources;

    RenderResourceList _on_surface_ready;
    RenderResourceList _on_every_frame;

    D_FList<sp<Updatable>> _on_pre_compose_updatable;
    D_FList<sp<Runnable>> _on_pre_compose_runnable;

    Vector<UpdatableSynchronized<bool>> _on_pre_render_sync;
    D_FList<sp<Runnable>> _on_pre_render_runnable;

    Vector<Box> _defered_instances;
    Map<uint32_t, sp<PrimitiveIndexBuffer>> _shared_primitive_index_buffer;

    GraphicsBufferAllocator _gba;

    friend class TextureBundle;
};

}
