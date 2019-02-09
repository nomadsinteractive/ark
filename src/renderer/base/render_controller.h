#ifndef ARK_RENDERER_BASE_RENDER_CONTROLLER_H_
#define ARK_RENDERER_BASE_RENDER_CONTROLLER_H_

#include <vector>

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/collection/filtered_list.h"
#include "core/collection/list.h"
#include "core/concurrent/lf_queue.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "renderer/forwarding.h"
#include "renderer/inf/uploader.h"
#include "renderer/util/named_buffer.h"

namespace ark {

class ARK_API RenderController {
public:
    enum UploadStrategy {
        US_ONCE = 0,
        US_RELOAD = 1,
        US_ON_SURFACE_READY = 2,
        US_ONCE_AND_ON_SURFACE_READY = 3
    };

public:
    RenderController(const sp<RenderEngine>& renderEngine, const sp<Recycler>& recycler, const sp<Dictionary<bitmap>>& bitmapLoader, const sp<Dictionary<bitmap>>& bitmapBoundsLoader);

    void reset();

    const sp<Recycler>& recycler() const;

    void onSurfaceReady(GraphicsContext& graphicsContext);
    void onSurfaceDestroy(GraphicsContext& graphicsContext);

    void onDrawFrame(GraphicsContext& graphicsContext);

    void upload(const sp<Resource>& resource, const sp<Uploader>& uploader, RenderController::UploadStrategy strategy);

    void uploadBuffer(const Buffer& buffer, const sp<Uploader>& uploader, RenderController::UploadStrategy strategy);

    template<typename T, typename... Args> sp<T> createResource(Args&&... args) {
        const sp<T> res = sp<T>::make(std::forward<Args>(args)...);
        upload(res, nullptr, RenderController::US_ONCE_AND_ON_SURFACE_READY);
        return res;
    }

    const sp<RenderEngine>& renderEngine() const;

    sp<PipelineFactory> createPipelineFactory() const;
    sp<Dictionary<sp<Texture>>> createTextureBundle() const;
    sp<Texture> createTexture(uint32_t width, uint32_t height, const sp<Variable<bitmap>>& bitmapVariable, UploadStrategy us = US_ONCE_AND_ON_SURFACE_READY);

    Buffer makeBuffer(Buffer::Type type, Buffer::Usage usage, const sp<Uploader>& uploader);
    Buffer makeVertexBuffer(Buffer::Usage usage = Buffer::USAGE_DYNAMIC, const sp<Uploader>& uploader = nullptr);
    Buffer makeIndexBuffer(Buffer::Usage usage = Buffer::USAGE_DYNAMIC, const sp<Uploader>& uploader = nullptr);

    sp<Framebuffer> makeFramebuffer(const sp<Renderer>& renderer, const sp<Texture>& texture);

    sp<Variable<uint64_t>> ticker() const;

    void addPreUpdateRequest(const sp<Runnable>& task, const sp<Boolean>& expired);

    void preUpdate();
    void deferUnref(const Box& box);

    const sp<NamedBuffer>& getNamedBuffer(NamedBuffer::Name name) const;

private:
    class ExpirableResource {
    public:
        ExpirableResource() = default;
        ExpirableResource(const sp<Resource>& resource, const sp<Uploader>& uploader);
        ExpirableResource(const ExpirableResource& other) = default;

        const sp<Resource>& resource() const;

        bool isExpired() const;

        void upload(GraphicsContext& graphicsContext) const;
        void recycle(GraphicsContext& graphicsContext) const;

        bool operator < (const ExpirableResource& other) const;

    private:
        sp<Resource> _resource;
        sp<Uploader> _uploader;
    };

    struct PreparingResource {
        PreparingResource() = default;
        PreparingResource(const ExpirableResource& resource, RenderController::UploadStrategy strategy);
        PreparingResource(const PreparingResource& other) = default;

        ExpirableResource _resource;
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
    std::set<ExpirableResource> _on_surface_ready_items;

    ListWithLifecycle<Runnable> _on_pre_update_request;
    std::vector<Box> _defered_instances;

    sp<NamedBuffer> _named_buffers[NamedBuffer::NAME_COUNT];
};

}

#endif
