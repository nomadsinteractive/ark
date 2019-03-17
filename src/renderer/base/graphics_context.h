#ifndef ARK_RENDERER_BASE_GRAPHICS_CONTEXT_H_
#define ARK_RENDERER_BASE_GRAPHICS_CONTEXT_H_

#include "core/base/api.h"
#include "core/collection/by_type.h"
#include "core/types/shared_ptr.h"
#include "core/types/weak_ptr.h"

#include "graphics/forwarding.h"

#include "renderer/forwarding.h"

namespace ark {

class ARK_API GraphicsContext {
public:
    GraphicsContext(const sp<RenderContext>& renderContext, const sp<RenderController>& renderController);
    ~GraphicsContext();

    void onSurfaceReady();
    void onDrawFrame();

    const sp<RenderContext>& renderContext() const;
    const sp<RenderController>& renderController() const;
    const sp<Recycler>& recycler() const;

    template<typename T> void attach(const sp<T>& attachment) {
        _attachments.put<T>(attachment);
    }
    template<typename T> const sp<T>& attachment() {
        return _attachments.ensure<T>();
    }

    uint32_t tick() const;

private:
    sp<RenderContext> _render_context;
    sp<RenderController> _render_controller;
    uint32_t _tick;

    ByType _attachments;

};

}

#endif
