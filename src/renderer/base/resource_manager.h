#ifndef ARK_RENDERER_BASE_RESOURCE_MANAGER_H_
#define ARK_RENDERER_BASE_RESOURCE_MANAGER_H_

#include <set>

#include "core/base/api.h"
#include "core/forwarding.h"
#include "core/types/owned_ptr.h"
#include "core/types/shared_ptr.h"
#include "core/concurrent/lf_stack.h"
#include "core/inf/dictionary.h"

#include "graphics/forwarding.h"

#include "renderer/forwarding.h"
#include "renderer/base/buffer.h"
#include "renderer/inf/uploader.h"

namespace ark {

class ARK_API ResourceManager {
public:
    ResourceManager(const sp<Dictionary<bitmap>>& bitmapLoader, const sp<Dictionary<bitmap>>& bitmapBoundsLoader);
    ~ResourceManager();

    enum UploadStrategy {
        US_ONCE = 0,
        US_RELOAD = 1,
        US_ON_SURFACE_READY = 2,
        US_ONCE_AND_ON_SURFACE_READY = 3,
        US_PRIORITY_HIGHT = 4
    };

    const sp<Dictionary<bitmap>>& bitmapLoader() const;
    const sp<Dictionary<bitmap>>& bitmapBoundsLoader() const;
    const sp<Recycler>& recycler() const;

    void onSurfaceReady(GraphicsContext& graphicsContext);
    void onSurfaceDestroy(GraphicsContext& graphicsContext);

    void onDrawFrame(GraphicsContext& graphicsContext);

    void upload(const sp<Resource>& resource, const sp<Uploader>& uploader, UploadStrategy strategy);

    void uploadBuffer(const Buffer& buffer, const sp<Uploader>& uploader, UploadStrategy strategy);

    template<typename T, typename... Args> sp<T> createResource(Args&&... args) {
        const sp<T> res = sp<T>::make(std::forward<Args>(args)...);
        upload(res, nullptr, US_ONCE_AND_ON_SURFACE_READY);
        return res;
    }

private:
    class ExpirableGLResource {
    public:
        ExpirableGLResource(const sp<Resource>& resource, const sp<Uploader>& uploader);
        ExpirableGLResource(const ExpirableGLResource& other) = default;

        const sp<Resource>& resource() const;

        bool isExpired() const;

        void upload(GraphicsContext& graphicsContext) const;
        void recycle(GraphicsContext& graphicsContext) const;

        bool operator < (const ExpirableGLResource& other) const;

    private:
        sp<Resource> _resource;
        sp<Uploader> _uploader;
    };

    struct PreparingGLResource {
        PreparingGLResource(const ExpirableGLResource& resource, UploadStrategy strategy);
        PreparingGLResource(const PreparingGLResource& other) = default;

        ExpirableGLResource _resource;
        UploadStrategy _strategy;

        bool operator < (const PreparingGLResource& other) const;
    };

private:
    void prepare(GraphicsContext& graphicsContext, LFStack<PreparingGLResource>& items);
    void doRecycling(GraphicsContext& graphicsContext);
    void doSurfaceReady(GraphicsContext& graphicsContext);

private:
    sp<Dictionary<bitmap>> _bitmap_loader;
    sp<Dictionary<bitmap>> _bitmap_bounds_loader;

    sp<Recycler> _recycler;

    LFStack<PreparingGLResource> _preparing_items[2];

    std::set<ExpirableGLResource> _on_surface_ready_items;

    friend class RenderController;
};

}

#endif
