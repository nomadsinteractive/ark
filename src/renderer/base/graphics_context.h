#ifndef ARK_RENDERER_BASE_GRAPHICS_CONTEXT_H_
#define ARK_RENDERER_BASE_GRAPHICS_CONTEXT_H_

#include "core/base/api.h"
#include "core/collection/by_type.h"
#include "core/types/shared_ptr.h"
#include "core/types/weak_ptr.h"

#include "graphics/forwarding.h"

#include "renderer/forwarding.h"
#include "renderer/opengl/base/gl_pipeline.h"

namespace ark {

class ARK_API GraphicsContext {
public:
    GraphicsContext(const sp<GLContext>& glContext, const sp<GLResourceManager>& resourceManager);
    ~GraphicsContext();

    void onSurfaceReady();
    void onDrawFrame();

    const sp<GLContext>& glContext() const;
    const sp<GLResourceManager>& resourceManager() const;

    template<typename T> const sp<T>& attachment() {
        return _attachments.ensure<T>();
    }

    uint64_t tick() const;

private:
    sp<GLContext> _gl_context;
    sp<GLResourceManager> _gl_resource_manager;
    sp<Variable<uint64_t>> _steady_clock;

    ByType _attachments;
    uint64_t _tick;

};

}

#endif
