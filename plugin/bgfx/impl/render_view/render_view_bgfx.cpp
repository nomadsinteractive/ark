#include "render_view_bgfx.h"

#include <bgfx/bgfx.h>

#include "graphics/base/color.h"
#include "graphics/inf/render_command.h"

#include "renderer/base/graphics_context.h"

#include "bgfx/base/bgfx_context.h"

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
    ::bgfx::reset(width, height);
}

void RenderViewBgfx::onRenderFrame(const Color& backgroundColor, RenderCommand& renderCommand)
{
    ::bgfx::touch(0);
    ::bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH | BGFX_CLEAR_STENCIL, backgroundColor.value());
    renderCommand.draw(_graphics_context);
    ::bgfx::frame();
}

}
