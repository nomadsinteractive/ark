#pragma once

#include "core/ark.h"
#include "core/types/shared_ptr.h"

#include "renderer/forwarding.h"
#include "renderer/inf/renderer_factory.h"

#include "renderer/vulkan/forward.h"

namespace ark::vulkan {

class RendererFactoryVulkan final : public RendererFactory {
public:
    RendererFactoryVulkan(sp<Recycler> recycler);
    ~RendererFactoryVulkan() override = default;

    sp<RenderEngineContext> createRenderEngineContext(const ApplicationManifest::Renderer& renderer) override;
    void onSurfaceCreated(RenderEngine& renderEngine) override;

    sp<Buffer::Delegate> createBuffer(Buffer::Type type, Buffer::Usage usage) override;
    sp<Camera::Delegate> createCamera(Ark::RendererCoordinateSystem rcs) override;
    sp<RenderTarget> createRenderTarget(sp<Renderer> renderer, RenderTarget::CreateConfigure configure) override;
    sp<RenderView> createRenderView(const sp<RenderEngineContext>& renderContext, const sp<RenderController>& renderController) override;
    sp<PipelineFactory> createPipelineFactory() override;
    sp<Texture::Delegate> createTexture(sp<Size> size, sp<Texture::Parameters> parameters) override;

private:
    sp<Recycler> _recycler;
    sp<VKRenderer> _renderer;

    friend class VKUtil;
    friend class PipelineFactoryVulkan;

};

}
