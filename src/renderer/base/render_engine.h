#pragma once

#include "core/ark.h"
#include "core/base/api.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "renderer/forwarding.h"

namespace ark {

class ARK_API RenderEngine {
public:
    RenderEngine(Ark::RendererVersion version, Ark::RendererCoordinateSystem coordinateSystem, sp<RendererFactory> rendererFactory);

    Ark::RendererVersion version() const;

    const sp<RendererFactory>& rendererFactory() const;
    const sp<RenderEngineContext>& context() const;

    const Viewport& viewport() const;

    float toLayoutDirection(float direction) const;
    bool isLHS() const;

    Rect toViewportRect(const Rect& rect, Ark::RendererCoordinateSystem cs = Ark::COORDINATE_SYSTEM_DEFAULT) const;
    V2 toViewportPosition(const V2& position, Ark::RendererCoordinateSystem cs = Ark::COORDINATE_SYSTEM_DEFAULT) const;
    Rect toRendererRect(const Rect& scissor, Ark::RendererCoordinateSystem cs = Ark::COORDINATE_SYSTEM_DEFAULT) const;
    V3 toWorldPosition(const M4& vpMatrix, float screenX, float screenY, float z) const;

    void onSurfaceCreated();

    sp<RenderView> createRenderView(const sp<RenderController>& renderController, const Viewport& viewport) const;

private:
    Ark::RendererCoordinateSystem _coordinate_system;

    sp<RendererFactory> _renderer_factory;
    sp<RenderEngineContext> _render_context;

};

}
