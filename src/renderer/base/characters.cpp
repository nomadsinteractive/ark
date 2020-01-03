#include "renderer/base/characters.h"

#include <cwctype>

#include "core/ark.h"
#include "core/inf/array.h"
#include "core/inf/variable.h"
#include "core/impl/boolean/boolean_by_weak_ref.h"
#include "core/util/math.h"

#include "graphics/base/layer.h"
#include "graphics/base/layer_context.h"
#include "graphics/base/render_object.h"
#include "graphics/base/size.h"
#include "graphics/base/v2.h"
#include "graphics/inf/character_mapper.h"
#include "graphics/inf/character_maker.h"
#include "graphics/util/vec3_util.h"

#include "renderer/base/atlas.h"
#include "renderer/base/render_engine.h"
#include "renderer/base/resource_loader_context.h"
#include "renderer/inf/render_model.h"

#include "app/base/application_context.h"
#include "app/view/layout_param.h"

namespace ark {

Characters::Characters(const sp<LayerContext>& layer, float textScale, float letterSpacing, float lineHeight, float lineIndent)
    : Characters(layer, nullptr, nullptr, textScale, letterSpacing, lineHeight, lineIndent)
{
}

Characters::Characters(const sp<Layer>& layer, float textScale, float letterSpacing, float lineHeight, float lineIndent)
    : Characters(layer->context(), textScale, letterSpacing, lineHeight, lineIndent)
{
}

Characters::Characters(const sp<RenderLayer>& layer, float textScale, float letterSpacing, float lineHeight, float lineIndent)
    : Characters(layer->makeContext(Layer::TYPE_DYNAMIC), textScale, letterSpacing, lineHeight, lineIndent)
{
}

Characters::Characters(const sp<LayerContext>& layerContext, const sp<CharacterMapper>& characterMapper, const sp<CharacterMaker>& characterMaker, float textScale, float letterSpacing, float lineHeight, float lineIndent)
    : _layer_context(layerContext), _character_mapper(characterMapper), _character_maker(characterMaker), _text_scale(textScale), _letter_spacing(letterSpacing),
      _layout_direction(Ark::instance().applicationContext()->renderEngine()->toLayoutDirection(1.0f)), _line_height(_layout_direction * lineHeight), _line_indent(lineIndent),
      _model(layerContext->renderModel()), _size(sp<Size>::make(0.0f, 0.0f))
{
}

const sp<LayoutParam>& Characters::layoutParam() const
{
    return _layout_param;
}

void Characters::setLayoutParam(const sp<LayoutParam>& layoutParam)
{
    _layout_param = layoutParam;
    if(_layout_param)
        _layout_size = _layout_param->size()->val();
}

const std::vector<sp<RenderObject>>& Characters::contents() const
{
    return _contents;
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
    createContent();
}

void Characters::renderRequest(const V3& position)
{
    if(_layout_param && _layout_size != _layout_param->size()->val())
    {
        _layout_size = _layout_param->size()->val();
        createContent();
    }

    _layer_context->renderRequest(V3());
    for(const sp<RenderablePassive>& i : _renderables)
        i->requestUpdate(position);
}

void Characters::createContent()
{
    for(const sp<RenderObject>& i : _contents)
        i->dispose();
    _contents.clear();

    float boundary = _layout_param ? _layout_param->contentWidth() : 0;
    if(boundary > 0)
        createContentWithBoundary(boundary);
    else
        createContentNoBoundary();
}

void Characters::createLayerContent(float width, float height)
{
    _size->setWidth(width);
    _size->setHeight(height);

    for(const sp<RenderObject>& i : _contents)
    {
        sp<RenderablePassive> renderable = sp<RenderablePassive>::make(i);
        _layer_context->add(renderable, sp<BooleanByWeakRef<RenderablePassive>>::make(renderable, 1));
        _renderables.push_back(std::move(renderable));
    }
}

void Characters::createContentWithBoundary(float boundary)
{
    float flowx = 0, flowy = getFlowY();
    const std::vector<LayoutChar> layoutChars = Characters::getCharacterMetrics(_text);
    float fontHeight = layoutChars.size() > 0 ? layoutChars.at(0)._metrics.bounds.y() * _text_scale : 0;
    size_t begin = 0;
    for(size_t i = 0; i < layoutChars.size(); ++i)
    {
        size_t end = i + 1;
        const LayoutChar& currentChar = layoutChars.at(i);
        if(end == layoutChars.size() || currentChar._is_cjk || currentChar._is_word_break)
        {
            if(end - begin == 1)
            {
                placeOne(currentChar, flowx, flowy);
                if(flowx > boundary || currentChar._is_line_break)
                    nextLine(fontHeight, flowx, flowy);
            }
            else
            {
                float beginWidth = begin > 0 ? layoutChars.at(begin - 1)._width_integral : 0;
                float width = currentChar._width_integral - beginWidth;
                if(flowx + width > boundary || currentChar._is_line_break)
                    nextLine(fontHeight, flowx, flowy);
                place(layoutChars, begin, end, flowx, flowy);
            }
            begin = i + 1;
        }
    }

    createLayerContent(flowx, std::abs(flowy) + fontHeight);
}

void Characters::createContentNoBoundary()
{
    float flowx = 0, flowy = getFlowY();
    float fontHeight = 0;
    for(wchar_t c : _text)
        placeNoBoundary(c, flowx, flowy, fontHeight);

    createLayerContent(flowx, std::abs(flowy) + fontHeight);
}

void Characters::placeNoBoundary(wchar_t c, float& flowx, float& flowy, float& fontHeight)
{
    int32_t type = toType(c);
    const Metrics metrics = _model->measure(type);
    float bitmapWidth = _text_scale * metrics.size.x();
    float bitmapHeight = _text_scale * metrics.size.y();
    float width = _text_scale * metrics.bounds.x();
    float height = _text_scale * metrics.bounds.y();
    float bitmapX = _text_scale * metrics.xyz.x();
    float bitmapY = _text_scale * metrics.xyz.y();
    const sp<Size> itemSize = sp<Size>::make(bitmapWidth, bitmapHeight);
    if(fontHeight == 0)
        fontHeight = height;
    else
        flowx += _letter_spacing;
    _contents.push_back(makeCharacter(type, V2(flowx + bitmapX, flowy + height - bitmapY - bitmapHeight), itemSize));
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
    const sp<Size> itemSize = sp<Size>::make(bitmapWidth, bitmapHeight);
    _contents.push_back(makeCharacter(layoutChar._type, V2(flowx + bitmapX, flowy + height - bitmapY - bitmapHeight), itemSize));
    flowx += width;
}

void Characters::nextLine(float fontHeight, float& flowx, float& flowy) const
{
    flowy += (_line_height != 0 ? _line_height : (fontHeight * _layout_direction));
    flowx = _line_indent;
}

float Characters::getFlowY() const
{
    if(!_layout_param || _layout_direction > 0)
        return 0;
    return _layout_param->size()->height() + _line_height;
}

std::vector<Characters::LayoutChar> Characters::getCharacterMetrics(const std::wstring& text) const
{
    std::vector<LayoutChar> metrics;
    std::unordered_map<wchar_t, std::tuple<Metrics, bool, bool>> mmap;
    float integral = 0;
    metrics.reserve(text.size());
    for(wchar_t c : text)
    {
        int32_t type = toType(c);
        bool isLineBreak = c == '\n';
        const auto iter = mmap.find(c);
        if(iter != mmap.end())
        {
            const std::tuple<Metrics, bool, bool>& val = iter->second;
            integral += _text_scale * std::get<0>(val).bounds.x();
            metrics.emplace_back(type, std::get<0>(val), integral, std::get<1>(val), std::get<2>(val), isLineBreak);
        }
        else
        {
            const Metrics m = _model->measure(type);
            bool iscjk = isCJK(c);
            bool iswordbreak = isWordBreaker(c);
            integral += _text_scale * m.bounds.x();
            mmap.insert(std::make_pair(c, std::make_tuple(m, iscjk, iswordbreak)));
            metrics.emplace_back(type, m, integral, iscjk, iswordbreak, isLineBreak);
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

int32_t Characters::toType(wchar_t c) const
{
    return _character_mapper ? _character_mapper->mapCharacter(static_cast<int32_t>(c)) : static_cast<int32_t>(c);
}

sp<RenderObject> Characters::makeCharacter(int32_t type, const V2& position, const sp<Size>& size) const
{
    return _character_maker ? _character_maker->makeCharacter(type, position, size) :
                              sp<RenderObject>::make(type, sp<Vec3>::make<Vec3::Const>(V3(position, 0)), size);
}

Characters::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _layer_context(sp<LayerContext::BUILDER>::make(factory, manifest, Layer::TYPE_DYNAMIC)), _character_mapper(factory.getBuilder<CharacterMapper>(manifest, "character-mapper")),
      _character_maker(factory.getBuilder<CharacterMaker>(manifest, "character-maker")),
      _text_scale(Documents::getAttribute<float>(manifest, "text-scale", 1.0f)), _letter_spacing(Documents::getAttribute<float>(manifest, "letter-spacing", 0.0f)),
      _line_height(Documents::getAttribute<float>(manifest, "line-height", 0.0f)), _line_indent(Documents::getAttribute<float>(manifest, "line-indent", 0.0f))
{
}

sp<Characters> Characters::BUILDER::build(const Scope& args)
{
    return sp<Characters>::make(_layer_context->build(args), _character_mapper->build(args), _character_maker->build(args), _text_scale, _letter_spacing, _line_height, _line_indent);
}

Characters::LayoutChar::LayoutChar(int32_t type, const Metrics& metrics, float widthIntegral, bool isCJK, bool isWordBreak, bool isLineBreak)
    : _type(type), _metrics(metrics), _width_integral(widthIntegral), _is_cjk(isCJK), _is_word_break(isWordBreak), _is_line_break(isLineBreak)
{
}

}
