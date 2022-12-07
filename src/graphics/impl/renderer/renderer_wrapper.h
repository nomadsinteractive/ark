#ifndef ARK_GRAPHICS_IMPL_RENDERER_RENDERER_WRAPPER_H_
#define ARK_GRAPHICS_IMPL_RENDERER_RENDERER_WRAPPER_H_

#include "core/forwarding.h"
#include "core/base/wrapper.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/inf/renderer.h"

namespace ark {

class RendererWrapper : public Renderer, public Wrapper<Renderer> {
public:
    RendererWrapper(const sp<Renderer>& delegate);

    virtual void render(RenderRequest& renderRequest, const V3& position) override;

};

}

#endif
