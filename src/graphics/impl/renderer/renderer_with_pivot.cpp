#include "graphics/impl/renderer/renderer_with_pivot.h"

#include "core/inf/variable.h"
#include "core/base/bean_factory.h"

#include "graphics/base/v2.h"
#include "graphics/base/size.h"
#include "graphics/inf/block.h"

namespace ark {

RendererWithPivot::RendererWithPivot(const sp<Renderer>& renderer, Pivot pivot)
    : _renderer(renderer), _block(renderer.as<Block>()), _pivot(pivot)
{
    DCHECK(_block, "Only block renderer may have pivot");
}

void RendererWithPivot::render(RenderRequest& renderRequest, float x, float y)
{
    float ox = 0, oy = 0;
    const sp<Size>& size = _block->size();

    if((_pivot & CENTER_HORIZONTAL) == CENTER_HORIZONTAL)
        ox = size->width() / -2.0f;
    else if((_pivot & RIGHT) == RIGHT)
        ox = -size->width();

    if((_pivot & CENTER_VERTICAL) == CENTER_VERTICAL)
        oy = size->height() / -2.0f;
    else if((_pivot & TOP) == TOP)
        oy = (-1.0 - g_upDirection) * size->height() / 2;
    else if((_pivot & BOTTOM) == BOTTOM)
        oy = (-1.0 + g_upDirection) * size->height() / 2;

    _renderer->render(renderRequest, x + ox, y + oy);
}

RendererWithPivot::STYLE::STYLE(BeanFactory& /*factory*/, const sp<Builder<Renderer>>& delegate, const String& value)
    : _delegate(delegate), _pivot(Strings::parse<RendererWithPivot::Pivot>(value))
{
}

sp<Renderer> RendererWithPivot::STYLE::build(const sp<Scope>& args)
{
    const sp<Renderer> bean = _delegate->build(args);
    return sp<Renderer>::adopt(new RendererWithPivot(bean, _pivot)).absorb(bean);
}

template<> RendererWithPivot::Pivot Conversions::to<String, RendererWithPivot::Pivot>(const String& s)
{
    uint32_t pivot = RendererWithPivot::NONE;
    for(const String& i : s.split('|'))
    {
        const String str = i.strip();
        if(str == "left")
            pivot |= RendererWithPivot::LEFT;
        else if(str == "right")
            pivot |= RendererWithPivot::RIGHT;
        else if(str == "top")
            pivot |= RendererWithPivot::TOP;
        else if(str == "bottom")
            pivot |= RendererWithPivot::BOTTOM;
        else if(str == "center")
            pivot |= RendererWithPivot::CENTER;
        else if(str == "center_horizontal")
            pivot |= RendererWithPivot::CENTER_HORIZONTAL;
        else if(str == "center_vertical")
            pivot |= RendererWithPivot::CENTER_VERTICAL;
    }
    return static_cast<RendererWithPivot::Pivot>(pivot);
}


}
