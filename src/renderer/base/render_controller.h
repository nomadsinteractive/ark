#ifndef ARK_RENDERER_BASE_RENDER_CONTROLLER_H_
#define ARK_RENDERER_BASE_RENDER_CONTROLLER_H_

#include <vector>
#include <unordered_map>

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/collection/list.h"
#include "core/concurrent/lf_queue.h"
#include "core/inf/runnable.h"
#include "core/inf/variable.h"
#include "core/impl/boolean/boolean_by_weak_ref.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "renderer/forwarding.h"
#include "renderer/base/buffer.h"
#include "renderer/base/texture.h"
#include "renderer/inf/model_loader.h"

#include "platform/platform.h"

namespace ark {

class ARK_API RenderController {
public:
//  [[script::bindings::enumeration]]
    enum UploadStrategy {
        US_RELOAD = 0,
        US_ONCE = 1,
        US_ON_SURFACE_READY = 2,
        US_ONCE_AND_ON_SURFACE_READY = 3,
        US_ON_CHANGE = 4,
        US_ON_EVERY_FRAME = 8
    };

//  [[script::bindings::enumeration]]
    enum UploadPriority {
        UPLOAD_PRIORITY_HIGH = 0,
        UPLOAD_PRIORITY_NORMAL,
        UPLOAD_PRIORITY_LOW,
        UPLOAD_PRIORITY_COUNT
    };

    enum SharedIndicesName {
        SHARED_INDICES_NONE,
        SHARED_INDICES_QUAD,
        SHARED_INDICES_NINE_PATCH,
        SHARED_INDICES_POINT,
        SHARED_INDICES_COUNT
    };

private:
    template<typename T> class SynchronizedVar : public Updatable {
    public:
        SynchronizedVar(sp<Variable<T>> delegate)
            : _delegate(std::move(delegate)), _var(sp<typename Variable<T>::Impl>::make(_delegate->val())) {
        }

        const sp<typename Variable<T>::Impl>& var() const {
            return _var;
        }

        virtual bool update(uint64_t timestamp) override {
            if(_delegate->update(timestamp)) {
                _var->set(_delegate->val());
                return true;
            }
            return false;
        }

    private:
        sp<Variable<T>> _delegate;
        sp<typename Variable<T>::Impl> _var;
    };

public:
    RenderController(const sp<RenderEngine>& renderEngine, const sp<Recycler>& recycler, const sp<Dictionary<bitmap>>& bitmapLoader, const sp<Dictionary<bitmap>>& bitmapBoundsLoader);

    void reset();

    const sp<Recycler>& recycler() const;

    void onSurfaceReady(GraphicsContext& graphicsContext);
    void onSurfaceDestroy(GraphicsContext& graphicsContext);

    void onDrawFrame(GraphicsContext& graphicsContext);

    void upload(sp<Resource> resource, RenderController::UploadStrategy strategy, sp<Updatable> updatable = nullptr, sp<Future> future = nullptr, UploadPriority priority = UPLOAD_PRIORITY_NORMAL);

//  [[script::bindings::auto]]
    void uploadBuffer(Buffer& buffer, sp<Input> input, RenderController::UploadStrategy strategy, sp<Future> future = nullptr, RenderController::UploadPriority priority = RenderController::UPLOAD_PRIORITY_NORMAL);

    const sp<RenderEngine>& renderEngine() const;

    sp<Camera> createCamera() const;
    sp<PipelineFactory> createPipelineFactory() const;

    sp<Texture> createTexture(sp<Size> size, sp<Texture::Parameters> parameters, sp<Texture::Uploader> uploader, RenderController::UploadStrategy us = US_ONCE_AND_ON_SURFACE_READY, sp<Future> future = nullptr);

//  [[script::bindings::auto]]
    sp<Texture> createTexture2D(sp<Size> size, sp<Bitmap> bitmap, RenderController::UploadStrategy us = RenderController::US_ONCE_AND_ON_SURFACE_READY, sp<Future> future = nullptr);

//  [[script::bindings::auto]]
    Buffer makeBuffer(Buffer::Type type, Buffer::Usage usage, sp<Input> uploader, RenderController::UploadStrategy us, sp<Future> future = nullptr);
    Buffer makeBuffer(Buffer::Type type, Buffer::Usage usage, sp<Input> input);
//  [[script::bindings::auto]]
    Buffer makeVertexBuffer(Buffer::Usage usage = Buffer::USAGE_DYNAMIC, sp<Input> input = nullptr);
//  [[script::bindings::auto]]
    Buffer makeIndexBuffer(Buffer::Usage usage = Buffer::USAGE_DYNAMIC, sp<Input> input = nullptr);

    sp<Framebuffer> makeFramebuffer(sp<Renderer> renderer, std::vector<sp<Texture>> colorAttachments, sp<Texture> depthStencilAttachments, int32_t clearMask);

    template<typename T> sp<Variable<T>> synchronize(sp<Variable<T>> delegate, sp<Boolean> disposed) {
        const sp<SynchronizedVar<T>> s = sp<SynchronizedVar<T>>::make(std::move(delegate));
        const sp<Variable<T>>& var = s->var();
        _on_pre_updatable.push_back(s, disposed ? std::move(disposed) : sp<Boolean>::make<BooleanByWeakRef<Variable<T>>>(var, 1));
        return var;
    }

    void addPreRenderUpdateRequest(sp<Updatable> updatable, sp<Boolean> disposed);
    void addPreRenderRunRequest(sp<Runnable> task, sp<Boolean> disposed);

    void preRequestUpdate(uint64_t timestamp);
    void deferUnref(Box box);

    sp<SharedIndices> getSharedIndices(SharedIndicesName name);
    sp<SharedIndices> getSharedIndices(const Model& model, bool degenerate);

    uint64_t updateTick();
    uint64_t tick() const;

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
        UploadingRenderResource(RenderResource resource, UploadStrategy strategy, UploadPriority priority);
        DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(UploadingRenderResource);

        RenderResource _resource;
        UploadStrategy _strategy;
        UploadPriority _priority;
    };

    class RenderResourceList {
    public:
        void append(UploadPriority priority, RenderResource ur);

        void foreach(GraphicsContext& graphicsContext, bool recycle, bool upload);

    private:
        std::vector<RenderResource> _resources[UPLOAD_PRIORITY_COUNT];
    };

private:
    void prepare(GraphicsContext& graphicsContext, LFQueue<UploadingRenderResource>& items);

private:
    sp<RenderEngine> _render_engine;
    sp<Recycler> _recycler;
    sp<Dictionary<bitmap>> _bitmap_loader;
    sp<Dictionary<bitmap>> _bitmap_bounds_loader;
    sp<Variable<uint64_t>> _clock;

    LFQueue<UploadingRenderResource> _uploading_resources;

    RenderResourceList _on_surface_ready;
    RenderResourceList _on_every_frame;

    DList<Updatable> _on_pre_updatable;
    DList<Runnable> _on_pre_update_request;

    std::vector<Box> _defered_instances;
    std::unordered_map<uint32_t, sp<SharedIndices>> _shared_indices;

    uint64_t _tick;

    friend class TextureBundle;
};

}

#endif
