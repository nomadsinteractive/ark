#include "render_view_bgfx.h"

#include <bgfx/bgfx.h>

#include "graphics/base/color.h"
#include "graphics/inf/render_command.h"

#include "renderer/base/graphics_context.h"

#include "bgfx/base/bgfx_context.h"
#include "renderer/base/render_engine_context.h"

namespace ark::plugin::bgfx {

RenderViewBgfx::RenderViewBgfx(sp<RenderEngineContext> renderContext, sp<RenderController> renderController):
    _graphics_context(new GraphicsContext(std::move(renderContext), std::move(renderController)))
{
}

void RenderViewBgfx::onSurfaceCreated()
{
}

void RenderViewBgfx::onSurfaceChanged(uint32_t width, uint32_t height)
{
    _bgfx_context = sp<BgfxContext>::make();

    _graphics_context.reset(new GraphicsContext(_graphics_context->renderContext(), _graphics_context->renderController()));
    _graphics_context->attachments().put(_bgfx_context);

    uint32_t flags = BGFX_RESET_NONE;
    const sp<RenderEngineContext>& renderContext = _graphics_context->renderContext();
    if(renderContext->renderer()._vsync)
        flags |= BGFX_RESET_VSYNC;
    ::bgfx::reset(width, height, flags);
}

void RenderViewBgfx::onRenderFrame(const Color& backgroundColor, RenderCommand& renderCommand)
{
    const RenderEngineContext::Resolution& displayResolution = _graphics_context->renderContext()->displayResolution();
    ::bgfx::setViewRect(0, 0, 0, static_cast<uint16_t>(displayResolution.width), static_cast<uint16_t>(displayResolution.height));
    ::bgfx::touch(0);
    ::bgfx::setState(BGFX_STATE_DEFAULT | BGFX_STATE_BLEND_FUNC_SEPARATE(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA, BGFX_STATE_BLEND_ONE, BGFX_STATE_BLEND_ZERO));
    ::bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH | BGFX_CLEAR_STENCIL, backgroundColor.value());
    _graphics_context->onDrawFrame();
    renderCommand.draw(_graphics_context);
    ::bgfx::frame();
}

}
