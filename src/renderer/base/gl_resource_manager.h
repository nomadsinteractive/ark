#ifndef ARK_RENDERER_BASE_GL_RESOURCE_MANAGER_H_
#define ARK_RENDERER_BASE_GL_RESOURCE_MANAGER_H_

#include <list>
#include <map>
#include <set>

#include "core/base/api.h"
#include "core/forwarding.h"
#include "core/types/owned_ptr.h"
#include "core/types/shared_ptr.h"
#include "core/concurrent/lock_free_stack.h"
#include "core/inf/dictionary.h"

#include "graphics/forwarding.h"

#include "renderer/base/gl_buffer.h"
#include "renderer/forwarding.h"

#include "platform/gl/gl.h"

namespace ark {

class ARK_API GLResourceManager {
public:
    GLResourceManager(const sp<Dictionary<bitmap>>& bitmapLoader, const sp<Dictionary<bitmap>>& bitmapBoundsLoader);
    ~GLResourceManager();

    enum BufferName {
        BUFFER_NAME_TRANGLES = 0,
        BUFFER_NAME_NINE_PATCH,
        BUFFER_NAME_POINTS,
        BUFFER_NAME_COUNT
    };

    enum PreparingStrategy {
        PS_ONCE,
        PS_ONCE_FORCE,
        PS_ON_SURFACE_READY,
        PS_ONCE_AND_ON_SURFACE_READY
    };

    void onSurfaceReady(GraphicsContext& graphicsContext);
    void onSurfaceDestroy(GraphicsContext& graphicsContext);

    void onDrawFrame(GraphicsContext& graphicsContext);

    void prepare(const sp<GLResource>& resource, PreparingStrategy strategy);
    void recycle(const sp<GLResource>& resource) const;

    GLBuffer getGLIndexBuffer(BufferName bufferName, uint32_t bufferLength);

    sp<GLTexture> loadGLTexture(const String& name);
    sp<GLTexture> createGLTexture(uint32_t width, uint32_t height, const sp<Variable<bitmap>>& bitmapVariable);
    GLBuffer createGLBuffer(const sp<Variable<bytearray>>& variable, GLenum type, GLenum usage);

    const sp<GLRecycler>& recycler() const;

    template<typename T, typename... Args> sp<GLResource> createGLResource(Args&&... args) {
        const sp<GLResource> res = sp<T>::make(std::forward<Args>(args)...);
        prepare(res, PS_ONCE_AND_ON_SURFACE_READY);
        return res;
    }

private:
    class ExpirableGLResource {
    public:
        ExpirableGLResource(const sp<GLResource>& resource);
        ExpirableGLResource(const ExpirableGLResource& other);

        const sp<GLResource>& resource() const;

        bool isExpired() const;

        void prepare(GraphicsContext& graphicsContext, bool force) const;
        void recycle(GraphicsContext& graphicsContext) const;

        bool operator < (const ExpirableGLResource& other) const;

    private:
        sp<GLResource> _resource;
    };

    class PreparingGLResource {
    public:
        PreparingGLResource(const ExpirableGLResource& resource, PreparingStrategy strategy);
        PreparingGLResource(const PreparingGLResource& other);

        ExpirableGLResource _resource;
        PreparingStrategy _strategy;

        bool operator < (const PreparingGLResource& other) const;
    };

private:
    GLBuffer createStaticBuffer(BufferName bufferName, uint32_t bufferLength) const;
    void doRecycling(GraphicsContext& graphicsContext);
    void doSurfaceReady(GraphicsContext& graphicsContext);

private:
    sp<GLRecycler> _recycler;
    sp<Dictionary<sp<GLTexture>>> _gl_texture_loader;

    LockFreeStack<PreparingGLResource> _preparing_items;

    std::set<ExpirableGLResource> _on_surface_ready_items;
    GLBuffer _static_buffers[BUFFER_NAME_COUNT];

    uint32_t _tick;
};

}

#endif
