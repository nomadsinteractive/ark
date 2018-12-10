#ifndef ARK_RENDERER_BASE_RESOURCE_MANAGER_H_
#define ARK_RENDERER_BASE_RESOURCE_MANAGER_H_

#include <set>

#include "core/base/api.h"
#include "core/forwarding.h"
#include "core/types/owned_ptr.h"
#include "core/types/shared_ptr.h"
#include "core/concurrent/lock_free_stack.h"
#include "core/inf/dictionary.h"

#include "graphics/forwarding.h"

#include "renderer/base/buffer.h"
#include "renderer/forwarding.h"

#include "platform/gl/gl.h"

namespace ark {

class ARK_API ResourceManager {
public:
    ResourceManager(const sp<Dictionary<bitmap>>& bitmapLoader, const sp<Dictionary<bitmap>>& bitmapBoundsLoader);
    ~ResourceManager();

    enum UploadStrategy {
        US_ONCE,
        US_ONCE_FORCE,
        US_ON_SURFACE_READY,
        US_ONCE_AND_ON_SURFACE_READY
    };

    const sp<Dictionary<bitmap>>& bitmapLoader() const;
    const sp<Dictionary<bitmap>>& bitmapBoundsLoader() const;

    void onSurfaceReady(GraphicsContext& graphicsContext);
    void onSurfaceDestroy(GraphicsContext& graphicsContext);

    void onDrawFrame(GraphicsContext& graphicsContext);

    void upload(const sp<Resource>& resource, UploadStrategy strategy);

    void prepare(const Buffer& buffer, UploadStrategy strategy);
    void recycle(const sp<Resource>& resource) const;

    sp<Texture> loadGLTexture(const String& name);
    sp<Texture> createGLTexture(uint32_t width, uint32_t height, const sp<Variable<bitmap>>& bitmapVariable, UploadStrategy ps = US_ONCE_AND_ON_SURFACE_READY);

//    Buffer makeBuffer(const sp<Buffer::Uploader>& uploader, Buffer::Type type, Buffer::Usage usage);
//    Buffer makeDynamicArrayBuffer() const;
//    Buffer::Snapshot makeBufferSnapshot(Buffer::Name name, const Buffer::UploadMakerFunc& maker, size_t reservedObjectCount, size_t size);

    const sp<Recycler>& recycler() const;

    template<typename T, typename... Args> sp<T> createGLResource(Args&&... args) {
        const sp<T> res = sp<T>::make(std::forward<Args>(args)...);
        upload(res, US_ONCE_AND_ON_SURFACE_READY);
        return res;
    }

private:
    class ExpirableGLResource {
    public:
        ExpirableGLResource(const sp<Resource>& resource);
        ExpirableGLResource(const ExpirableGLResource& other);

        const sp<Resource>& resource() const;

        bool isExpired() const;

        void prepare(GraphicsContext& graphicsContext) const;
        void recycle(GraphicsContext& graphicsContext) const;

        bool operator < (const ExpirableGLResource& other) const;

    private:
        sp<Resource> _resource;
    };

    struct PreparingGLResource {
        PreparingGLResource(const ExpirableGLResource& resource, UploadStrategy strategy);
        PreparingGLResource(const PreparingGLResource& other);

        ExpirableGLResource _resource;
        UploadStrategy _strategy;

        bool operator < (const PreparingGLResource& other) const;
    };

    struct SharedBuffer {
        Buffer _buffers[Buffer::NAME_COUNT];
    };

private:
    void doRecycling(GraphicsContext& graphicsContext);
    void doSurfaceReady(GraphicsContext& graphicsContext);

private:
    sp<Dictionary<bitmap>> _bitmap_loader;
    sp<Dictionary<bitmap>> _bitmap_bounds_loader;

    sp<Recycler> _recycler;
    sp<Dictionary<sp<Texture>>> _gl_texture_loader;

    LockFreeStack<PreparingGLResource> _preparing_items;
    LockFreeStack<sp<SharedBuffer>> _shared_buffers;

    std::set<ExpirableGLResource> _on_surface_ready_items;

    uint32_t _tick;

    friend class RenderController;
};

}

#endif
