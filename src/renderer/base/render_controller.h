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
#include "graphics/base/camera.h"

#include "renderer/forwarding.h"
#include "renderer/base/texture.h"
#include "renderer/base/shared_buffer.h"
#include "renderer/inf/model_loader.h"

#include "platform/platform.h"

namespace ark {

class ARK_API RenderController {
private:
    template<typename T> class SynchronizedVar : public Updatable {
    public:
        SynchronizedVar(const sp<Variable<T>>& delegate)
            : _delegate(delegate), _var(sp<typename Variable<T>::Impl>::make(_delegate->val())) {
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
    enum UploadStrategy {
        US_ONCE = 0,
        US_RELOAD = 1,
        US_ON_SURFACE_READY = 2,
        US_ONCE_AND_ON_SURFACE_READY = 3
    };

    enum UploadPriority {
        UP_DEFAULT,
        UP_LEVEL_1,
        UP_LEVEL_2
    };

public:
    RenderController(const sp<RenderEngine>& renderEngine, const sp<Recycler>& recycler, const sp<Dictionary<bitmap>>& bitmapLoader, const sp<Dictionary<bitmap>>& bitmapBoundsLoader);

    void reset();

    const sp<Recycler>& recycler() const;

    void onSurfaceReady(GraphicsContext& graphicsContext);
    void onSurfaceDestroy(GraphicsContext& graphicsContext);

    void onDrawFrame(GraphicsContext& graphicsContext);

    void upload(const sp<Resource>& resource, const sp<Uploader>& uploader, RenderController::UploadStrategy strategy, UploadPriority priority = UP_DEFAULT);
    void uploadBuffer(const Buffer& buffer, const sp<Uploader>& uploader, RenderController::UploadStrategy strategy, UploadPriority priority = UP_DEFAULT);

    template<typename T, typename... Args> sp<T> createResource(Args&&... args) {
        const sp<T> res = sp<T>::make(std::forward<Args>(args)...);
        upload(res, nullptr, RenderController::US_ONCE_AND_ON_SURFACE_READY);
        return res;
    }

    const sp<RenderEngine>& renderEngine() const;

    sp<Camera::Delegate> createCamera() const;
    sp<PipelineFactory> createPipelineFactory() const;

    sp<Texture> createTexture(sp<Size> size, sp<Texture::Parameters> parameters, sp<Texture::Uploader> uploader, RenderController::UploadStrategy us = US_ONCE_AND_ON_SURFACE_READY);
    sp<Texture> createTexture2D(sp<Size> size, sp<Texture::Uploader> uploader, UploadStrategy us = US_ONCE_AND_ON_SURFACE_READY);

    Buffer makeBuffer(Buffer::Type type, Buffer::Usage usage, const sp<Uploader>& uploader);
    Buffer makeVertexBuffer(Buffer::Usage usage = Buffer::USAGE_DYNAMIC, const sp<Uploader>& uploader = nullptr);
    Buffer makeIndexBuffer(Buffer::Usage usage = Buffer::USAGE_DYNAMIC, const sp<Uploader>& uploader = nullptr);

    sp<Framebuffer> makeFramebuffer(sp<Renderer> renderer, std::vector<sp<Texture>> textures, int32_t clearMask);

    template<typename T> sp<Variable<T>> synchronize(const sp<Variable<T>>& delegate, const sp<Boolean>& disposed) {
        const sp<SynchronizedVar<T>> s = sp<SynchronizedVar<T>>::make(delegate);
        const sp<Variable<T>>& var = s->var();
        _on_pre_updatable.push_back(s, disposed ? disposed : sp<Boolean>::make<BooleanByWeakRef<Variable<T>>>(var, 1));
        return var;
    }

    void addPreUpdateRequest(const sp<Runnable>& task, const sp<Boolean>& expired);

    void preUpdate(uint64_t timestamp);
    void deferUnref(Box box);

    sp<SharedBuffer> getNamedBuffer(SharedBuffer::Name name);
    sp<SharedBuffer> getSharedBuffer(ModelLoader::RenderMode renderMode, const Model& model);

private:
    class RenderResource {
    public:
        RenderResource() = default;
        RenderResource(const sp<Resource>& resource, const sp<Uploader>& uploader, UploadPriority uploadPriority);
        RenderResource(const RenderResource& other) = default;

        const sp<Resource>& resource() const;

        bool isExpired() const;

        void upload(GraphicsContext& graphicsContext) const;
        void recycle(GraphicsContext& graphicsContext) const;

        UploadPriority uploadPriority() const;

        bool operator < (const RenderResource& other) const;

    private:
        sp<Resource> _resource;
        sp<Uploader> _uploader;
        UploadPriority _upload_priority;
    };

    struct PreparingResource {
        PreparingResource() = default;
        PreparingResource(const RenderResource& resource, RenderController::UploadStrategy strategy);
        PreparingResource(const PreparingResource& other) = default;

        RenderResource _resource;
        RenderController::UploadStrategy _strategy;

        bool operator < (const PreparingResource& other) const;
    };

private:
    void prepare(GraphicsContext& graphicsContext, LFQueue<PreparingResource>& items);
    void doRecycling(GraphicsContext& graphicsContext);
    void doSurfaceReady(GraphicsContext& graphicsContext);

    element_index_t getIndicesHash(Uploader& indices) const;

private:
    sp<RenderEngine> _render_engine;
    sp<Recycler> _recycler;
    sp<Dictionary<bitmap>> _bitmap_loader;
    sp<Dictionary<bitmap>> _bitmap_bounds_loader;

    LFQueue<PreparingResource> _preparing_items;
    std::set<RenderResource> _on_surface_ready_items;

    DisposableItemList<Updatable> _on_pre_updatable;
    DisposableItemList<Runnable> _on_pre_update_request;

    std::vector<Box> _defered_instances;
    std::unordered_map<element_index_t, sp<SharedBuffer>> _shared_buffers;

    friend class TextureBundle;
};

}

#endif
