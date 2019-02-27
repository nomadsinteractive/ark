#include "renderer/base/characters.h"

#include <cwctype>

#include "core/ark.h"
#include "core/inf/array.h"
#include "core/inf/variable.h"
#include "core/util/math.h"

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

    float boundary = _layout_param ? _layout_param->contentWidth() : 0;
    if(boundary > 0)
        createContent(boundary);
    else
        createContentNoBoundary();
}

void Characters::renderRequest(const V2& position)
{
    _layer_context->renderRequest(position);
}

void Characters::createContent(float boundary)
{
    float flowx = 0, flowy = 0;
    const std::vector<LayoutChar> layoutChars = Characters::getCharacterMetrics(_text);
    float fontHeight = layoutChars.size() > 0 ? layoutChars.at(0)._metrics.bounds.y() * _text_scale : 0;
    size_t begin = 0;
    for(size_t i = 0; i < layoutChars.size(); ++i)
    {
        size_t end = i + 1;
        const LayoutChar& currentChar = layoutChars.at(i);
        if(end == layoutChars.size() || currentChar._is_cjk || isWordBreaker(currentChar._char))
        {
            if(end - begin == 1)
            {
                placeOne(currentChar, flowx, flowy);
                if(flowx > boundary)
                    nextLine(fontHeight, flowx, flowy);
            }
            else
            {
                float beginWidth = begin > 0 ? layoutChars.at(begin - 1)._width_integral : 0;
                float width = currentChar._width_integral - beginWidth;
                if(flowx + width > boundary)
                    nextLine(fontHeight, flowx, flowy);
                place(layoutChars, begin, end, flowx, flowy);
            }
            begin = i + 1;
        }
    }

    _size->setWidth(flowx);
    _size->setHeight(std::abs(flowy) + fontHeight);

    _layer_context->clear();
    for(const sp<RenderObject>& i : _characters)
        _layer_context->addRenderObject(i);
}

Metrics Characters::getItemMetrics(wchar_t c) const
{
    return _model->measure(c);
}

void Characters::createContentNoBoundary()
{
    float flowx = 0, flowy = 0;
    float fontHeight = 0;
    for(wchar_t c : _text)
        placeNoBoundary(c, flowx, flowy, fontHeight);
    _size->setWidth(flowx);
    _size->setHeight(std::abs(flowy) + fontHeight);

    _layer_context->clear();
    for(const sp<RenderObject>& i : _characters)
        _layer_context->addRenderObject(i);
}

void Characters::placeNoBoundary(wchar_t c, float& flowx, float& flowy, float& fontHeight)
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
    _characters.push_back(_object_pool->obtain<RenderObject>(c, _object_pool->obtain<Vec::Const>(V(flowx + bitmapX, flowy + height - bitmapY - bitmapHeight)), itemSize));
    flowx += width;
}

void Characters::place(const std::vector<Characters::LayoutChar>& layouts, size_t begin, size_t end, float& flowx, float flowy)
{
    for(size_t i = begin; i < end; ++i)
    {
        if(begin > 0)
            flowx += _letter_spacing;

        const Characters::LayoutChar& layoutChar = layouts.at(i);
        placeOne(layoutChar, flowx, flowy);
    }
}

void Characters::placeOne(const Characters::LayoutChar& layoutChar, float& flowx, float flowy)
{
    const Metrics& metrics = layoutChar._metrics;
    float bitmapWidth = _text_scale * metrics.size.x();
    float bitmapHeight = _text_scale * metrics.size.y();
    float width = _text_scale * metrics.bounds.x();
    float height = _text_scale * metrics.bounds.y();
    float bitmapX = _text_scale * metrics.xyz.x();
    float bitmapY = _text_scale * metrics.xyz.y();
    const sp<Size> itemSize = _object_pool->obtain<Size>(bitmapWidth, bitmapHeight);
    _characters.push_back(_object_pool->obtain<RenderObject>(layoutChar._char, _object_pool->obtain<Vec::Const>(V(flowx + bitmapX, flowy + height - bitmapY - bitmapHeight)), itemSize));
    flowx += width;
}

void Characters::nextLine(float fontHeight, float& flowx, float& flowy) const
{
    flowy += (_line_height != 0 ? _line_height : (-fontHeight * g_upDirection));
    flowx = _line_indent;
}

std::vector<Characters::LayoutChar> Characters::getCharacterMetrics(const std::wstring& text) const
{
    std::vector<LayoutChar> metrics;
    std::unordered_map<wchar_t, std::pair<Metrics, bool>> mmap;
    float integral = 0;
    metrics.reserve(text.size());
    for(wchar_t c : text)
    {
        const auto iter = mmap.find(c);
        if(iter != mmap.end())
        {
            const std::pair<Metrics, bool>& val = iter->second;
            integral += _text_scale * val.first.bounds.x();
            metrics.emplace_back(c, val.first, integral, val.second);
        }
        else
        {
            const Metrics m = _model->measure(c);
            bool iscjk = isCJK(c);
            integral += _text_scale * m.bounds.x();
            mmap.insert(std::make_pair(c, std::make_pair(m, iscjk)));
            metrics.emplace_back(c, m, integral, iscjk);
        }
    }
    return metrics;
}

bool Characters::isCJK(int32_t c) const
{
    return c == 0x3005 || Math::between<int32_t>(0x3400, 0x4DBF, c) || Math::between<int32_t>(0x4E00, 0x9FFF, c) ||
           Math::between<int32_t>(0xF900, 0xFAFF, c) || Math::between<int32_t>(0x20000, 0x2A6DF, c) || Math::between<int32_t>(0x2A700, 0x2B73F, c) ||
            Math::between<int32_t>(0x2B740, 0x2B81F, c) || Math::between<int32_t>(0x2F800, 0x2FA1F, c);
}

bool Characters::isWordBreaker(wchar_t c) const
{
    return c != '_' && !std::iswalpha(c);
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

Characters::LayoutChar::LayoutChar(wchar_t c, const Metrics& metrics, float widthIntegral, bool isCJK)
    : _char(c), _metrics(metrics), _width_integral(widthIntegral), _is_cjk(isCJK)
{
}

}
