#include "renderer/base/characters.h"

#include <cwctype>

#include "core/ark.h"
#include "core/inf/array.h"
#include "core/inf/variable.h"
#include "core/impl/boolean/boolean_by_weak_ref.h"
#include "core/util/bean_utils.h"
#include "core/util/math.h"
#include "core/util/text_type.h"

#include "graphics/base/layer.h"
#include "graphics/base/layer_context.h"
#include "graphics/base/glyph.h"
#include "graphics/base/render_layer.h"
#include "graphics/base/render_object.h"
#include "graphics/base/render_request.h"
#include "graphics/base/size.h"
#include "graphics/base/v3.h"
#include "graphics/impl/glyph_maker/glyph_maker_span.h"
#include "graphics/impl/renderable/renderable_passive.h"
#include "graphics/inf/glyph_maker.h"

#include "renderer/base/atlas.h"
#include "renderer/base/model.h"
#include "renderer/base/render_engine.h"
#include "renderer/base/resource_loader_context.h"
#include "renderer/inf/model_loader.h"

#include "app/base/application_context.h"
#include "app/base/resource_loader.h"
#include "app/view/layout_param.h"

#include <tinyxml2.h>


namespace ark {

Characters::Characters(const sp<LayerContext>& layer, sp<Text> text, float textScale, float letterSpacing, float lineHeight, float lineIndent)
    : Characters(Ark::instance().applicationContext()->resourceLoader()->beanFactory(), layer, std::move(text), nullptr, textScale, letterSpacing, lineHeight, lineIndent)
{
}

Characters::Characters(const sp<Layer>& layer, sp<Text> text, float textScale, float letterSpacing, float lineHeight, float lineIndent)
    : Characters(layer->context(), std::move(text), textScale, letterSpacing, lineHeight, lineIndent)
{
}

Characters::Characters(const BeanFactory& factory, const sp<LayerContext>& layerContext, sp<Text> text, const sp<GlyphMaker>& glyphMaker, float textScale, float letterSpacing, float lineHeight, float lineIndent)
    : _bean_factory(factory), _layer_context(layerContext), _text(text ? std::move(text) : TextType::create()), _text_scale(textScale), _glyph_maker(glyphMaker ? glyphMaker : sp<GlyphMaker>::make<GlyphMakerSpan>()),
      _letter_spacing(letterSpacing), _layout_direction(Ark::instance().applicationContext()->renderEngine()->toLayoutDirection(1.0f)), _line_height(_layout_direction * lineHeight),
      _line_indent(lineIndent), _model_loader(layerContext->modelLoader()), _size(sp<Size>::make(0.0f, 0.0f))
{
    if(_text->val() && !_text->val()->empty())
        setText(Strings::fromUTF8(*_text->val()));
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
    return _text_unicode;
}

void Characters::setText(const std::wstring& text)
{
    _text_unicode = text;
    createContent();
}

void Characters::setRichText(const std::wstring& richText, const Scope& args)
{
    _text_unicode = richText;
    createRichContent(args);
}

void Characters::render(RenderRequest& renderRequest, const V3& position)
{
    if(_text->update(renderRequest.timestamp()))
        setText(Strings::fromUTF8(*_text->val()));
    else if(_layout_param && _layout_size != _layout_param->size()->val())
    {
        _layout_size = _layout_param->size()->val();
        layoutContent();
    }

    _layer_context->renderRequest(V3());
    for(const sp<RenderablePassive>& i : _renderables)
        i->requestUpdate(position);
}

void Characters::createContent()
{
    float boundary = _layout_param ? _layout_param->contentWidth() : 0;
    float flowx = boundary > 0 ? 0 : -_letter_spacing, flowy = getFlowY();
    _glyphs = makeGlyphs(_glyph_maker, _text_unicode);
    flowy = doLayoutContent(_glyphs, flowx, flowy, boundary);
    createLayerContent(flowx, flowy);
}

float Characters::doLayoutContent(GlyphContents& cm, float& flowx, float& flowy, float boundary)
{
    return boundary > 0 ? doLayoutWithBoundary(cm, flowx, flowy, boundary) : doLayoutWithoutBoundary(cm, flowx, getFlowY());
}

void Characters::createRichContent(const Scope& args)
{
    float boundary = _layout_param ? _layout_param->contentWidth() : 0;
    float flowx = boundary > 0 ? 0 : -_letter_spacing, flowy = getFlowY();
    BeanFactory factory = _bean_factory.ensure();
    const document richtext = Documents::parseFull(Strings::toUTF8(_text_unicode));
    const sp<GlyphMaker> characterMaker = factory.ensure<GlyphMaker>(richtext, args);
    _glyphs.clear();
    float height = doCreateRichContent(_glyphs, _glyph_maker, richtext, factory, args, flowx, flowy, boundary);
    createLayerContent(flowx, height);
}

float Characters::doCreateRichContent(GlyphContents& cm, GlyphMaker& gm, const document& richtext, BeanFactory& factory, const Scope& args, float& flowx, float& flowy, float boundary)
{
    float height = 0;
    for(const document& i : richtext->children())
    {
        if(i->type() == DOMElement::ELEMENT_TYPE_TEXT)
        {
            for(sp<Glyph> i : makeGlyphs(gm, Strings::fromUTF8(i->value())))
                cm.push_back(std::move(i));
        }
        else if(i->type() == DOMElement::ELEMENT_TYPE_ELEMENT)
        {
            const sp<GlyphMaker> characterMaker = factory.ensure<GlyphMaker>(i, args);
            height = doCreateRichContent(cm, characterMaker, i, factory, args, flowx, flowy, boundary);
        }
    }
    return height;
}

void Characters::layoutContent()
{
    DCHECK(_contents.size() == _text_unicode.length(), "Contents have changed, cannot do relayout");
    float boundary = _layout_param ? _layout_param->contentWidth() : 0;
    float flowx = boundary > 0 ? 0 : -_letter_spacing, flowy = getFlowY();
    flowy = doLayoutContent(_glyphs, flowx, flowy, boundary);
    for(size_t i = 0; i < _contents.size(); ++i)
        _contents.at(i)->setPosition(_glyphs.at(i)->toRenderObjectPosition());
    _size->setWidth(flowx);
    _size->setHeight(flowy);
}

void Characters::createLayerContent(float width, float height)
{
    _contents.clear();
    for(const sp<Glyph>& i : _glyphs)
        _contents.push_back(i->toRenderObject());

    _size->setWidth(width);
    _size->setHeight(height);

    _renderables.clear();
    for(const sp<RenderObject>& i : _contents)
    {
        sp<RenderablePassive> renderable = sp<RenderablePassive>::make(i);
        _layer_context->add(renderable, sp<BooleanByWeakRef<RenderablePassive>>::make(renderable, 1));
        _renderables.push_back(std::move(renderable));
    }
}

float Characters::doLayoutWithBoundary(GlyphContents& cm, float& flowx, float& flowy, float boundary)
{
    const std::vector<LayoutChar> layoutChars = Characters::getCharacterMetrics(cm);
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
                placeOne(cm[i], currentChar._metrics, flowx, flowy);
                if(flowx > boundary || currentChar._is_line_break)
                    nextLine(fontHeight, flowx, flowy);
            }
            else
            {
                float beginWidth = begin > 0 ? layoutChars.at(begin - 1)._width_integral : 0;
                float width = currentChar._width_integral - beginWidth;
                if(flowx + width > boundary || currentChar._is_line_break)
                    nextLine(fontHeight, flowx, flowy);
                place(cm, layoutChars, begin, end, flowx, flowy);
            }
            begin = i + 1;
        }
    }

    return std::abs(flowy) + fontHeight;
}

float Characters::doLayoutWithoutBoundary(GlyphContents& cm, float& flowx, float flowy)
{
    float fontHeight = 0;
    for(Glyph& i : cm)
    {
        const sp<Model> model = _model_loader->loadModel(i.type()->val());
        const Metrics& metrics = model->metrics();
        flowx += _letter_spacing;
        placeOne(i, metrics, flowx, flowy, &fontHeight);
    }
    return std::abs(flowy) + fontHeight;
}

void Characters::place(GlyphContents& cm, const std::vector<Characters::LayoutChar>& layouts, size_t begin, size_t end, float& flowx, float flowy)
{
    for(size_t i = begin; i < end; ++i)
    {
        if(begin > 0)
            flowx += _letter_spacing;

        const Characters::LayoutChar& layoutChar = layouts.at(i);
        placeOne(cm[i], layoutChar._metrics, flowx, flowy);
    }
}

void Characters::placeOne(Glyph& glyph, const Metrics& metrics, float& flowx, float flowy, float* fontHeight)
{
    const V2 scale = V2(_text_scale, _text_scale);
    float bitmapWidth = scale.x() * metrics.size.x();
    float bitmapHeight = scale.y() * metrics.size.y();
    float width = scale.x() * metrics.bounds.x();
    float height = scale.y() * metrics.bounds.y();
    float bitmapX = scale.x() * metrics.orgin.x();
    float bitmapY = scale.y() * metrics.orgin.y();
    if(fontHeight)
        *fontHeight = std::max(height, *fontHeight);
    glyph.setLayoutPosition(V3(flowx + bitmapX, flowy + height - bitmapY - bitmapHeight, 0));
    glyph.setLayoutSize(V2(bitmapWidth, bitmapHeight));
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

std::vector<Characters::LayoutChar> Characters::getCharacterMetrics(const GlyphContents& glyphs) const
{
    std::vector<LayoutChar> metrics;
    std::unordered_map<wchar_t, std::tuple<Metrics, bool, bool>> mmap;
    const float xScale = _text_scale;
    float integral = 0;
    metrics.reserve(glyphs.size());
    for(const sp<Glyph>& i : glyphs)
    {
        const wchar_t c = i->character();
        const bool isLineBreak = c == '\n';
        const auto iter = mmap.find(c);
        if(iter != mmap.end())
        {
            const std::tuple<Metrics, bool, bool>& val = iter->second;
            integral += xScale * std::get<0>(val).bounds.x();
            metrics.emplace_back(std::get<0>(val), integral, std::get<1>(val), std::get<2>(val), isLineBreak);
        }
        else
        {
            int32_t type = static_cast<int32_t>(c);
            const sp<Model> model = _model_loader->loadModel(type);
            const Metrics& m = model->metrics();
            bool iscjk = isCJK(c);
            bool iswordbreak = isWordBreaker(c);
            integral += xScale * m.bounds.x();
            mmap.insert(std::make_pair(c, std::make_tuple(m, iscjk, iswordbreak)));
            metrics.emplace_back(m, integral, iscjk, iswordbreak, isLineBreak);
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

Characters::GlyphContents Characters::makeGlyphs(GlyphMaker& gm, const std::wstring& text)
{
    Characters::GlyphContents glyphs = gm.makeGlyphs(text);
    DCHECK(glyphs.size() == text.size(), "Bad GlyphMaker result returned, size mismatch(%d, %d), text: %s", glyphs.size(), text.size(), Strings::toUTF8(text).c_str());
    for(size_t i = 0; i < text.size(); ++i)
        glyphs[i]->setCharacter(text.at(i));
    return glyphs;
}

Characters::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _bean_factory(factory), _text(factory.getBuilder<Text>(manifest, Constants::Attributes::TEXT)), _layer_context(sp<LayerContext::BUILDER>::make(factory, manifest, Layer::TYPE_DYNAMIC)),
      _glyph_maker(factory.getBuilder<GlyphMaker>(manifest, "glyph-maker")), _text_scale(factory.getBuilder<String>(manifest, "text-scale")), _letter_spacing(factory.getBuilder<Numeric>(manifest, "letter-spacing")),
      _line_height(Documents::getAttribute<float>(manifest, "line-height", 0.0f)), _line_indent(Documents::getAttribute<float>(manifest, "line-indent", 0.0f))
{
}

sp<Characters> Characters::BUILDER::build(const Scope& args)
{
    float textScale = _text_scale ? Strings::parse<float>(_text_scale->build(args)) : 1.0f;
    return sp<Characters>::make(_bean_factory, _layer_context->build(args), _text->build(args), _glyph_maker->build(args), textScale, BeanUtils::toFloat(_letter_spacing, args, 0.0f), _line_height, _line_indent);
}

Characters::LayoutChar::LayoutChar(const Metrics& metrics, float widthIntegral, bool isCJK, bool isWordBreak, bool isLineBreak)
    : _metrics(metrics), _width_integral(widthIntegral), _is_cjk(isCJK), _is_word_break(isWordBreak), _is_line_break(isLineBreak)
{
}

}
