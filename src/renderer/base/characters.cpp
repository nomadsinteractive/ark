#include "renderer/base/characters.h"

#include "core/ark.h"
#include "core/inf/array.h"
#include "core/inf/variable.h"

#include "graphics/base/layer_context.h"
#include "graphics/base/render_object.h"
#include "graphics/base/size.h"
#include "graphics/base/v2.h"

#include "renderer/base/atlas.h"
#include "renderer/base/resource_loader_context.h"
#include "renderer/inf/render_model.h"
#include "renderer/impl/layer/alphabet_layer.h"

#include "app/view/layout_param.h"

namespace ark {

Characters::Characters(const sp<Layer>& layer, float textScale, float letterSpacing, float lineHeight, float lineIndent)
    : Characters(layer, nullptr, textScale, letterSpacing, lineHeight, lineIndent)
{
}

Characters::Characters(const sp<Layer>& layer, const sp<ObjectPool>& objectPool, float textScale, float letterSpacing, float lineHeight, float lineIndent)
    : _layer(layer), _object_pool(objectPool ? objectPool : Ark::instance().objectPool()), _layer_context(layer->makeContext()),
      _text_scale(textScale), _letter_spacing(letterSpacing), _line_height(-g_upDirection * lineHeight),
      _line_indent(lineIndent), _model(layer->model()), _size(_object_pool->obtain<Size>(0.0f, 0.0f))
{
}

const sp<Layer>& Characters::layer() const
{
    return _layer;
}

const sp<LayoutParam>& Characters::layoutParam() const
{
    return _layout_param;
}

void Characters::setLayoutParam(const sp<LayoutParam>& layoutParam)
{
    _layout_param = layoutParam;
}

const std::vector<sp<RenderObject>>& Characters::characters() const
{
    return _characters;
}

const SafePtr<Size>& Characters::size() const
{
    return _size;
}

const std::wstring& Characters::text() const
{
    return _text;
}

void Characters::setText(const std::wstring& text)
{
    _text = text;
    _characters.clear();
    createContent();
}

void Characters::renderRequest(const V2& position)
{
    _layer_context->renderRequest(position);
}

Metrics Characters::getItemMetrics(wchar_t c) const
{
    return _model->measure(c);
}

void Characters::createContent()
{
    float flowx = 0, flowy = 0, boundary = _layout_param ? _layout_param->contentWidth() : 0;
    float fontHeight = 0;
    for(wchar_t c : _text)
        place(boundary, c, flowx, flowy, fontHeight);
    _size->setWidth(flowx);
    _size->setHeight(abs(flowy) + fontHeight);

    _layer_context->clear();
    for(const sp<RenderObject>& i : _characters)
        _layer_context->addRenderObject(i);
}

void Characters::place(float boundary, wchar_t c, float& flowx, float& flowy, float& fontHeight)
{
    const Metrics metrics = getItemMetrics(c);
    float bitmapWidth = _text_scale * metrics.size.x();
    float bitmapHeight = _text_scale * metrics.size.y();
    float width = _text_scale * metrics.bounds.x();
    float height = _text_scale * metrics.bounds.y();
    float bitmapX = _text_scale * metrics.xyz.x();
    float bitmapY = _text_scale * metrics.xyz.y();
    const sp<Size> itemSize = _object_pool->obtain<Size>(bitmapWidth, bitmapHeight);
    if(fontHeight == 0)
        fontHeight = height;
    else
        flowx += _letter_spacing;
    if(0 != boundary)
    {
        if(flowx + width > boundary)
        {
            flowy += (_line_height == 0 ? _line_height : (-fontHeight * g_upDirection));
            flowx = _line_indent;
        }
    }
    _characters.push_back(_object_pool->obtain<RenderObject>(c, _object_pool->obtain<Vec::Const>(V(flowx + bitmapX, flowy + height - bitmapY - bitmapHeight)), itemSize));
    flowx += width;
}

Characters::BUILDER::BUILDER(BeanFactory& factory, const document manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _layer(factory.ensureBuilder<Layer>(manifest, Constants::Attributes::LAYER)), _object_pool(resourceLoaderContext->objectPool()),
      _text_scale(Documents::getAttribute<float>(manifest, "text-scale", 1.0f)), _letter_spacing(Documents::getAttribute<float>(manifest, "letter-spacing", 0.0f)),
      _line_height(Documents::getAttribute<float>(manifest, "line-height", 0.0f)), _line_indent(Documents::getAttribute<float>(manifest, "line-indent", 0.0f))
{
}

sp<Characters> Characters::BUILDER::build(const sp<Scope>& args)
{
    return sp<Characters>::make(_layer->build(args), _object_pool, _text_scale, _letter_spacing, _line_height, _line_indent);
}

}
