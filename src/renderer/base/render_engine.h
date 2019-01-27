#ifndef ARK_RENDERER_INF_RENDER_ENGINE_H_
#define ARK_RENDERER_INF_RENDER_ENGINE_H_

#include "core/ark.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "renderer/forwarding.h"

namespace ark {

class RenderEngine {
public:
    RenderEngine(Ark::RendererVersion version, const sp<RendererFactory>& rendererFactory);

    Ark::RendererVersion version() const;

    const Viewport& viewport() const;
    const V2& resolution() const;
    void setResolution(const V2& resolution);

    const sp<RendererFactory>& rendererFactory() const;
    const sp<RenderContext>& renderContext() const;

    void onSurfaceCreated();

    sp<RenderView> createRenderView(const Viewport& viewport) const;

private:
    sp<RendererFactory> _renderer_factory;

    sp<RenderContext> _render_context;
};

}

#endif
