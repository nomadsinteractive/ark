#pragma once

#include "core/base/api.h"
#include "core/base/bit_set_types.h"

#include "graphics/forwarding.h"
#include "graphics/base/camera.h"

#include "renderer/forwarding.h"
#include "renderer/base/buffer.h"
#include "renderer/base/render_target.h"
#include "renderer/base/texture.h"

#include "app/base/application_manifest.h"

namespace ark {

class ARK_API RendererFactory {
public:
    struct Features {
        enums::RenderingBackendSet _supported_backends;
        enums::CoordinateSystem _default_coordinate_system;
        bool _can_draw_element_incremental;
        uint32_t _attribute_alignment;
    };

    RendererFactory(const Features& features)
        : _features(features) {
    }
    virtual ~RendererFactory() = default;

    [[nodiscard]]
    const Features& features() const {
        return _features;
    }

    virtual void onSurfaceCreated(RenderEngine& renderEngine) = 0;

    virtual sp<RenderEngineContext> createRenderEngineContext(const ApplicationManifest::Renderer& renderer) = 0;
    virtual sp<Buffer::Delegate> createBuffer(Buffer::Usage usage) = 0;
    virtual sp<Camera::Delegate> createCamera(enums::CoordinateSystem cs = enums::COORDINATE_SYSTEM_DEFAULT) = 0;
    virtual sp<RenderTarget> createRenderTarget(sp<Renderer> renderer, RenderTarget::Configure configure) = 0;
    virtual sp<PipelineFactory> createPipelineFactory() = 0;
    virtual sp<RenderView> createRenderView(const sp<RenderEngineContext>& renderContext, const sp<RenderController>& renderController) = 0;
    virtual sp<Texture::Delegate> createTexture(sp<Size> size, sp<Texture::Parameters> parameters) = 0;

protected:
    Features _features;
};

}
