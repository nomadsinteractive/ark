#ifndef ARK_RENDERER_BASE_RENDER_CONTROLLER_H_
#define ARK_RENDERER_BASE_RENDER_CONTROLLER_H_

#include <vector>

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
#include "renderer/inf/uploader.h"
#include "renderer/util/named_buffer.h"

namespace ark {

class ARK_API RenderController {
private:
    template<typename T> class Synchronized : public Runnable {
    public:
        Synchronized(const sp<Variable<T>>& delegate, const sp<Boolean>& flag)
            : _delegate(delegate), _flag(flag), _value(sp<typename Variable<T>::Impl>::make(_delegate->val())) {
        }

        virtual void run() override {
            if(_flag->val())
                _value->set(_delegate->val());
        }

        const sp<typename Variable<T>::Impl>& value() const {
            return _value;
        }

    private:
        sp<Variable<T>> _delegate;
        sp<Boolean> _flag;

        sp<typename Variable<T>::Impl> _value;

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

    sp<Texture> createTexture(const sp<Size>& size, const sp<Texture::Parameters>& parameters, const sp<Texture::Uploader>& uploader, RenderController::UploadStrategy us = US_ONCE_AND_ON_SURFACE_READY);
    sp<Texture> createTexture2D(const sp<Size>& size, const sp<Texture::Uploader>& uploader, UploadStrategy us = US_ONCE_AND_ON_SURFACE_READY);

    Buffer makeBuffer(Buffer::Type type, Buffer::Usage usage, const sp<Uploader>& uploader);
    Buffer makeVertexBuffer(Buffer::Usage usage = Buffer::USAGE_DYNAMIC, const sp<Uploader>& uploader = nullptr);
    Buffer makeIndexBuffer(Buffer::Usage usage = Buffer::USAGE_DYNAMIC, const sp<Uploader>& uploader = nullptr);

    sp<Framebuffer> makeFramebuffer(const sp<Renderer>& renderer, const sp<Texture>& texture);

    sp<Boolean> makeSynchronizeFlag();

    template<typename T> sp<Variable<T>> synchronize(const sp<Variable<T>>& delegate) {
        const sp<Synchronized<T>> s = sp<Synchronized<T>>::make(delegate, makeSynchronizeFlag());
        const sp<Variable<T>> var = s->value();
        addPreUpdateRequest(s, sp<BooleanByWeakRef<Variable<T>>>::make(var, 1));
        return var;
    }

    sp<Variable<uint64_t>> ticker() const;

    void addPreUpdateRequest(const sp<Runnable>& task, const sp<Boolean>& expired);

    void preUpdate();
    void deferUnref(Box box);

    const sp<NamedBuffer>& getNamedBuffer(NamedBuffer::Name name) const;

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

    class SynchronizeFlag : public Boolean {
    public:
        SynchronizeFlag();

        virtual bool val() override;

        void reset();

    private:
        bool _value;
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

private:
    sp<RenderEngine> _render_engine;
    sp<Recycler> _recycler;
    sp<Dictionary<bitmap>> _bitmap_loader;
    sp<Dictionary<bitmap>> _bitmap_bounds_loader;

    LFQueue<PreparingResource> _preparing_items;
    std::set<RenderResource> _on_surface_ready_items;

    DisposableItemList<Runnable> _on_pre_update_request;
    std::vector<Box> _defered_instances;
    WeakRefList<SynchronizeFlag> _synchronize_flags;

    sp<NamedBuffer> _named_buffers[NamedBuffer::NAME_COUNT];

    friend class TextureBundle;
};

}

#endif
