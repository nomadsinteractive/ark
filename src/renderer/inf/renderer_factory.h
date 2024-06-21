#pragma once

#include "core/ark.h"
#include "core/base/api.h"

#include "graphics/forwarding.h"
#include "graphics/base/camera.h"

#include "renderer/forwarding.h"
#include "renderer/base/buffer.h"
#include "renderer/base/texture.h"

namespace ark {

class ARK_API RendererFactory {
public:
    RendererFactory(Ark::RendererCoordinateSystem defaultCoordinateSystem)
        : _default_coordinate_system(defaultCoordinateSystem) {
    }
    virtual ~RendererFactory() = default;

    [[nodiscard]]
    Ark::RendererCoordinateSystem defaultCoordinateSystem() const {
        return _default_coordinate_system;
    }

    virtual sp<RenderEngineContext> initialize(Ark::RendererVersion version) = 0;

    virtual void onSurfaceCreated(RenderEngineContext& renderContext) = 0;

    virtual sp<Buffer::Delegate> createBuffer(Buffer::Type type, Buffer::Usage usage) = 0;
    virtual sp<Camera::Delegate> createCamera(Ark::RendererCoordinateSystem cs) = 0;
    virtual sp<Framebuffer> createFramebuffer(sp<Renderer> renderer, std::vector<sp<Texture>> colorAttachments, sp<Texture> depthStencilAttachments, int32_t clearMask) = 0;
    virtual sp<PipelineFactory> createPipelineFactory() = 0;
    virtual sp<RenderView> createRenderView(const sp<RenderEngineContext>& renderContext, const sp<RenderController>& renderController) = 0;
    virtual sp<Texture::Delegate> createTexture(sp<Size> size, sp<Texture::Parameters> parameters) = 0;

protected:
    Ark::RendererCoordinateSystem _default_coordinate_system;
};

}
