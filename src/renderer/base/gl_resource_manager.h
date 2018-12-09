#ifndef ARK_RENDERER_BASE_GL_RESOURCE_MANAGER_H_
#define ARK_RENDERER_BASE_GL_RESOURCE_MANAGER_H_

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

class ARK_API GLResourceManager {
public:
    GLResourceManager(const sp<Dictionary<bitmap>>& bitmapLoader, const sp<Dictionary<bitmap>>& bitmapBoundsLoader);
    ~GLResourceManager();

    enum PreparingStrategy {
        PS_ONCE,
        PS_ONCE_FORCE,
        PS_ON_SURFACE_READY,
        PS_ONCE_AND_ON_SURFACE_READY
    };

    const sp<Dictionary<bitmap>>& bitmapLoader() const;
    const sp<Dictionary<bitmap>>& bitmapBoundsLoader() const;

    void onSurfaceReady(GraphicsContext& graphicsContext);
    void onSurfaceDestroy(GraphicsContext& graphicsContext);

    void onDrawFrame(GraphicsContext& graphicsContext);

    void prepare(const sp<RenderResource>& resource, PreparingStrategy strategy);
    void prepare(const Buffer& buffer, PreparingStrategy strategy);
    void recycle(const sp<RenderResource>& resource) const;

    sp<Texture> loadGLTexture(const String& name);
    sp<Texture> createGLTexture(uint32_t width, uint32_t height, const sp<Variable<bitmap>>& bitmapVariable, PreparingStrategy ps = PS_ONCE_AND_ON_SURFACE_READY);

    Buffer makeGLBuffer(const sp<Buffer::Uploader>& uploader, GLenum type, GLenum usage);
    Buffer makeDynamicArrayBuffer() const;
    Buffer::Snapshot makeGLBufferSnapshot(Buffer::Name name, const Buffer::UploadMakerFunc& maker, size_t reservedObjectCount, size_t size);

    const sp<GLRecycler>& recycler() const;

    template<typename T, typename... Args> sp<T> createGLResource(Args&&... args) {
        const sp<T> res = sp<T>::make(std::forward<Args>(args)...);
        prepare(res, PS_ONCE_AND_ON_SURFACE_READY);
        return res;
    }

private:
    class ExpirableGLResource {
    public:
        ExpirableGLResource(const sp<RenderResource>& resource);
        ExpirableGLResource(const ExpirableGLResource& other);

        const sp<RenderResource>& resource() const;

        bool isExpired() const;

        void prepare(GraphicsContext& graphicsContext) const;
        void recycle(GraphicsContext& graphicsContext) const;

        bool operator < (const ExpirableGLResource& other) const;

    private:
        sp<RenderResource> _resource;
    };

    struct PreparingGLResource {
        PreparingGLResource(const ExpirableGLResource& resource, PreparingStrategy strategy);
        PreparingGLResource(const PreparingGLResource& other);

        ExpirableGLResource _resource;
        PreparingStrategy _strategy;

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

    sp<GLRecycler> _recycler;
    sp<Dictionary<sp<Texture>>> _gl_texture_loader;

    LockFreeStack<PreparingGLResource> _preparing_items;
    LockFreeStack<sp<SharedBuffer>> _shared_buffers;

    std::set<ExpirableGLResource> _on_surface_ready_items;

    uint32_t _tick;
};

}

#endif
