#include "graphics/impl/frame/bar.h"

#include "core/base/bean_factory.h"
#include "core/util/math.h"
#include "core/util/bean_utils.h"

#include "graphics/base/size.h"
#include "graphics/base/render_layer.h"
#include "graphics/base/render_object.h"
#include "graphics/base/v2.h"
#include "graphics/util/vec2_util.h"

namespace ark {

Bar::Bar(const sp<Layer>& layer, const sp<RenderObject>& bottom, const sp<RenderObject>& bolierplate, const sp<RenderObject>& top, const sp<Vec>& direction, const sp<Size>& size)
    : _bottom(bottom), _boilerplate(bolierplate), _top(top), _direction(direction), _size(size), _render_layer(sp<RenderLayer>::make(layer)), _bar_width(0), _bar_height(0)
{
}

void Bar::render(RenderRequest& renderRequest, float x, float y)
{
    update();
    _render_layer->render(renderRequest, x, y);
}

const sp<Size>& Bar::size()
{
    return _size;
}

void Bar::update()
{
    if(_size->width() != _bar_width)
    {
        _bar_width = _size->width();

        float bpw = _boilerplate->size()->width();
        float bph = _boilerplate->size()->height();
        float bodyLength = _bar_width - (_bottom ? _bottom->size()->width() : 0.0f) - (_top ? _top->size()->width() : 0.0f);

        if(bodyLength < 0)
            return;

        uint32_t count = static_cast<uint32_t>(bodyLength / bpw);
        float clip = bodyLength - count * bpw;
        _render_layer->clear();

        const V direction = _direction->val();
        float dx = direction.x() * bpw;
        float fx = 0.0f;
        const sp<Size>& boilerSize = _boilerplate->size();
        if(_bottom)
        {
            _render_layer->addRenderObject(_bottom);
            fx += _bottom->size()->width();
        }
        for(uint32_t i = 0; i < count; ++i)
        {
            const sp<RenderObject> cell = sp<RenderObject>::make(_boilerplate->type(), Vec2Util::create(fx, 0.0f), boilerSize, _boilerplate->transform(), _boilerplate->varyings());
            _render_layer->addRenderObject(cell);
            fx += dx;
        }
        if(clip != 0)
        {
            const sp<RenderObject> cell = sp<RenderObject>::make(_boilerplate->type(), Vec2Util::create(fx, 0.0f), sp<Size>::make(clip, bph), _boilerplate->transform(), _boilerplate->varyings());
            _render_layer->addRenderObject(cell);
            fx += clip;
        }
        if(_top)
        {
//            _top->setX(fx);
            _render_layer->addRenderObject(_top);
        }
    }
}

Bar::BUILDER::BUILDER(BeanFactory& parent, const document& doc)
    : _layer(parent.ensureBuilder<Layer>(doc, Constants::Attributes::LAYER)),
      _size(parent.ensureBuilder<Size>(doc, Constants::Attributes::SIZE)),
      _bottom(parent.getBuilder<RenderObject>(doc, "bottom")),
      _top(parent.getBuilder<RenderObject>(doc, "top")),
      _boilerplate(parent.ensureBuilder<RenderObject>(doc, "boilerplate")),
      _direction(parent.ensureBuilder<Vec>(doc, "direction"))
{
}

sp<Renderer> Bar::BUILDER::build(const sp<Scope>& args)
{
    return sp<Bar>::make(_layer->build(args), _bottom->build(args), _boilerplate->build(args), _top->build(args), _direction->build(args), _size->build(args));
}

}
