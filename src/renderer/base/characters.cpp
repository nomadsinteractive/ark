#include "renderer/base/characters.h"

#include "core/inf/array.h"
#include "core/impl/array/dynamic_array.h"
#include "core/inf/variable.h"

#include "graphics/base/render_object.h"
#include "graphics/base/size.h"
#include "graphics/base/v2.h"

#include "renderer/base/atlas.h"
#include "renderer/base/resource_loader_context.h"
#include "renderer/impl/layer/alphabet_layer.h"
#include "renderer/impl/layer/image_layer.h"

namespace ark {

Characters::Characters(const sp<Layer>& layer, float textScale, float letterSpacing, float lineHeight, float lineIndent)
    : Characters(layer, nullptr, textScale, letterSpacing, lineHeight, lineIndent)
{
}

Characters::Characters(const sp<Layer>& layer, const sp<ResourceLoaderContext>& resourceLoaderContext, float textScale, float letterSpacing, float lineHeight, float lineIndent)
    : _layer(layer), _object_pool(resourceLoaderContext ? resourceLoaderContext->objectPool() : nullptr), _text_scale(textScale), _letter_spacing(letterSpacing), _line_height(g_isOriginBottom ? -lineHeight : lineHeight),
      _line_indent(lineIndent), _x(0), _y(0), _width(0), _height(0), _size(sp<Size>::make(0.0f, 0.0f))
{
    _alphabet_layer = layer.as<AlphabetLayer>();
    if(_alphabet_layer)
        _alphabet = _alphabet_layer->alphabet();
    else
    {
        DCHECK(layer.is<ImageLayer>(), "Character's layer must be either AlphabetLayer or ImageLayer");
        _atlas = layer.as<ImageLayer>()->atlas();
    }
}

const sp<Layer>& Characters::layer() const
{
    return _layer;
}

const List<sp<RenderObject>>& Characters::characters() const
{
    return _characters;
}

const sp<Size>& Characters::size() const
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

void Characters::setBounds(float x, float y, float width, float height)
{
    _x = x;
    _y = y;
    _width = width;
    _height = height;
}

Alphabet::Metrics Characters::getItemMetrics(wchar_t c) const
{
    Alphabet::Metrics metrics;
    if(_atlas)
    {
        const Atlas::Item& item = _atlas->at(c);
        metrics.bitmap_width = metrics.width = static_cast<int32_t>(item.width() * _text_scale);
        metrics.bitmap_height = metrics.height = static_cast<int32_t>(item.height() * _text_scale);
        metrics.bitmap_x = metrics.bitmap_y = 0;
    }
    else
    {
        _alphabet->measure(c, metrics, false);
        if(_text_scale != 1.0f)
        {
            metrics.bitmap_width *= _text_scale;
            metrics.bitmap_height *= _text_scale;
            metrics.width *= _text_scale;
            metrics.height *= _text_scale;
            metrics.bitmap_x *= _text_scale;
            metrics.bitmap_y *= _text_scale;
        }
    }
    return metrics;
}

void Characters::createContent()
{
    float flowx = _x, flowy = _y, boundary = _x + _width;
    float fontHeight = 0;
    for(wchar_t c : _text)
        place(boundary, c, flowx, flowy, fontHeight);
    _size->setWidth(flowx - _x);
    _size->setHeight(abs(flowy - _y) + fontHeight);
}

void Characters::place(float boundary, wchar_t c, float& flowx, float& flowy, float& fontHeight)
{
    const Alphabet::Metrics metrics = getItemMetrics(c);
    const sp<Size> itemSize = _object_pool ? _object_pool->obtain<Size>(metrics.bitmap_width, metrics.bitmap_height)
                                           : sp<Size>::make(metrics.bitmap_width, metrics.bitmap_height);
    if(fontHeight == 0)
        fontHeight = static_cast<float>(metrics.height);
    else
        flowx += _letter_spacing;
    if(_x != boundary)
    {
        if(flowx + metrics.width > boundary)
        {
            flowy += (_line_height ? _line_height : (-fontHeight * g_upDirection));
            flowx = _x + _line_indent;
        }
    }
    _characters.push_back(sp<RenderObject>::make(c, sp<VV::Const>::make(V(flowx + metrics.bitmap_x, flowy + metrics.height - metrics.bitmap_y - metrics.bitmap_height)), itemSize));
    flowx += metrics.width;
}

Characters::BUILDER::BUILDER(BeanFactory& factory, const document manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _layer(factory.ensureBuilder<Layer>(manifest, Constants::Attributes::LAYER)), _text(Strings::load(manifest, Constants::Attributes::TEXT)), _resource_loader_context(resourceLoaderContext),
      _text_scale(Documents::getAttribute<float>(manifest, "text-scale", 1.0f)), _letter_spacing(Documents::getAttribute<float>(manifest, "letter-spacing", 0.0f)),
      _line_height(Documents::getAttribute<float>(manifest, "line-height", 0.0f)), _line_indent(Documents::getAttribute<float>(manifest, "line-indent", 0.0f))
{
}

sp<Characters> Characters::BUILDER::build(const sp<Scope>& args)
{
    const sp<Layer> layer = _layer->build(args);
    const sp<String> text = _text->build(args);
    const sp<Characters> characters = sp<Characters>::make(layer, _resource_loader_context, _text_scale, _letter_spacing, _line_height, _line_indent);
    if(text)
        characters->setText(Strings::fromUTF8(text));
    return characters;
}

}
