#include "renderer/base/text.h"

#include <cwctype>

#include "core/ark.h"
#include "core/inf/array.h"
#include "core/inf/variable.h"
#include "core/impl/boolean/boolean_by_weak_ref.h"
#include "core/util/bean_utils.h"
#include "core/util/math.h"
#include "core/util/string_type.h"

#include "graphics/base/layer.h"
#include "graphics/base/layer_context.h"
#include "graphics/base/glyph.h"
#include "graphics/base/render_layer.h"
#include "graphics/base/render_layer_snapshot.h"
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

Text::Text(const sp<LayerContext>& layer, sp<StringVar> text, float textScale, float letterSpacing, float lineHeight, float lineIndent)
    : Text(Ark::instance().applicationContext()->resourceLoader()->beanFactory(), nullptr, layer, std::move(text), nullptr, textScale, letterSpacing, lineHeight, lineIndent)
{
}

Text::Text(sp<RenderLayer> renderLayer, sp<StringVar> text, float textScale, float letterSpacing, float lineHeight, float lineIndent)
    : Text(renderLayer->context(), std::move(text), textScale, letterSpacing, lineHeight, lineIndent)
{
    _render_layer = std::move(renderLayer);
}

Text::Text(const sp<Layer>& layer, sp<StringVar> text, float textScale, float letterSpacing, float lineHeight, float lineIndent)
    : Text(layer->context(), std::move(text), textScale, letterSpacing, lineHeight, lineIndent)
{
}

Text::Text(const BeanFactory& factory, sp<RenderLayer> renderLayer, const sp<LayerContext>& layerContext, sp<StringVar> text, const sp<GlyphMaker>& glyphMaker, float textScale, float letterSpacing, float lineHeight, float lineIndent)
    : _bean_factory(factory), _render_layer(std::move(renderLayer)), _layer_context(layerContext), _text(text ? std::move(text) : StringType::create()), _text_scale(textScale),
      _glyph_maker(glyphMaker ? glyphMaker : sp<GlyphMaker>::make<GlyphMakerSpan>()), _letter_spacing(letterSpacing), _layout_direction(Ark::instance().applicationContext()->renderEngine()->toLayoutDirection(1.0f)),
      _line_height(_layout_direction * lineHeight), _line_indent(lineIndent), _model_loader(layerContext->modelLoader()), _size(sp<Size>::make(0.0f, 0.0f)), _content(sp<Content>::make())
{
    WARN(renderLayer, "Text without a RenderLayer will no longer be supported");
    if(_text->val() && !_text->val()->empty())
        setText(Strings::fromUTF8(*_text->val()));
}

const sp<LayoutParam>& Text::layoutParam() const
{
    return _layout_param;
}

void Text::setLayoutParam(const sp<LayoutParam>& layoutParam)
{
    _layout_param = layoutParam;
    if(_layout_param)
        _layout_size = _layout_param->size()->val();
}

const std::vector<sp<RenderObject>>& Text::contents() const
{
    return _content->renderObjects();
}

const sp<Vec3>& Text::position() const
{
    return _content->position().ensure();
}

void Text::setPosition(sp<Vec3> position)
{
    _content->position().reset(std::move(position));
}

const SafePtr<Size>& Text::size() const
{
    return _size;
}

const std::wstring& Text::text() const
{
    return _text_unicode;
}

void Text::setText(const std::wstring& text)
{
    _text_unicode = text;
    createContent();
}

void Text::show(sp<Boolean> disposed)
{
    _layer_context = _render_layer->makeContext(_content, nullptr, nullptr, std::move(disposed));
}

void Text::setRichText(const std::wstring& richText, const Scope& args)
{
    _text_unicode = richText;
    createRichContent(args);
}

void Text::render(RenderRequest& renderRequest, const V3& position)
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

void Text::createContent()
{
    float boundary = _layout_param ? _layout_param->contentWidth() : 0;
    float flowx = boundary > 0 ? 0 : -_letter_spacing, flowy = getFlowY();
    _glyphs = makeGlyphs(_glyph_maker, _text_unicode);
    flowy = doLayoutContent(_glyphs, flowx, flowy, boundary);
    createLayerContent(flowx, flowy);
}

float Text::doLayoutContent(GlyphContents& cm, float& flowx, float& flowy, float boundary)
{
    return boundary > 0 ? doLayoutWithBoundary(cm, flowx, flowy, boundary) : doLayoutWithoutBoundary(cm, flowx, getFlowY());
}

void Text::createRichContent(const Scope& args)
{
    float boundary = _layout_param ? _layout_param->contentWidth() : 0;
    float flowx = boundary > 0 ? 0 : -_letter_spacing, flowy = getFlowY();
    BeanFactory factory = _bean_factory.ensure();
    const document richtext = Documents::parseFull(Strings::toUTF8(_text_unicode));
    _glyphs.clear();
    float height = doCreateRichContent(_glyphs, _glyph_maker, richtext, factory, args, flowx, flowy, boundary);
    createLayerContent(flowx, height);
}

float Text::doCreateRichContent(GlyphContents& cm, GlyphMaker& gm, const document& richtext, BeanFactory& factory, const Scope& args, float& flowx, float& flowy, float boundary)
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

void Text::layoutContent()
{
    DCHECK(_content->renderObjects().size() == _text_unicode.length(), "Contents have changed, cannot do relayout");
    float boundary = _layout_param ? _layout_param->contentWidth() : 0;
    float flowx = boundary > 0 ? 0 : -_letter_spacing, flowy = getFlowY();
    flowy = doLayoutContent(_glyphs, flowx, flowy, boundary);
    for(size_t i = 0; i < _content->renderObjects().size(); ++i)
        _content->renderObjects().at(i)->setPosition(_glyphs.at(i)->toRenderObjectPosition());
    _size->setWidth(flowx);
    _size->setHeight(flowy);
}

void Text::createLayerContent(float width, float height)
{
    std::vector<sp<RenderObject>> renderObjects;

    for(const sp<Glyph>& i : _glyphs)
        renderObjects.push_back(i->toRenderObject());

    _size->setWidth(width);
    _size->setHeight(height);

    _renderables.clear();
    for(const sp<RenderObject>& i : renderObjects)
    {
        sp<RenderablePassive> renderable = sp<RenderablePassive>::make(i);
        _layer_context->add(renderable, sp<BooleanByWeakRef<RenderablePassive>>::make(renderable, 1));
        _renderables.push_back(std::move(renderable));
    }

    _content->setRenderObjects(std::move(renderObjects));
}

float Text::doLayoutWithBoundary(GlyphContents& cm, float& flowx, float& flowy, float boundary)
{
    const std::vector<LayoutChar> layoutChars = Text::getCharacterMetrics(cm);
    float fontHeight = layoutChars.size() > 0 ? layoutChars.at(0)._metrics.aabb().y() * _text_scale : 0;
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

float Text::doLayoutWithoutBoundary(GlyphContents& cm, float& flowx, float flowy)
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

void Text::place(GlyphContents& cm, const std::vector<Text::LayoutChar>& layouts, size_t begin, size_t end, float& flowx, float flowy)
{
    for(size_t i = begin; i < end; ++i)
    {
        if(begin > 0)
            flowx += _letter_spacing;

        const Text::LayoutChar& layoutChar = layouts.at(i);
        placeOne(cm[i], layoutChar._metrics, flowx, flowy);
    }
}

void Text::placeOne(Glyph& glyph, const Metrics& metrics, float& flowx, float flowy, float* fontHeight)
{
    const V2 scale = V2(_text_scale, _text_scale);
    float bitmapWidth = scale.x() * metrics.width();
    float bitmapHeight = scale.y() * metrics.height();
    float width = scale.x() * metrics.aabb().x();
    float height = scale.y() * metrics.aabb().y();
    float bitmapX = scale.x() * metrics.origin().x();
    float bitmapY = scale.y() * metrics.origin().y();
    if(fontHeight)
        *fontHeight = std::max(height, *fontHeight);
    glyph.setLayoutPosition(V3(flowx + bitmapX, flowy + height - bitmapY - bitmapHeight, 0));
    glyph.setLayoutSize(V2(bitmapWidth, bitmapHeight));
    flowx += width;
}

void Text::nextLine(float fontHeight, float& flowx, float& flowy) const
{
    flowy += (_line_height != 0 ? _line_height : (fontHeight * _layout_direction));
    flowx = _line_indent;
}

float Text::getFlowY() const
{
    if(!_layout_param || _layout_direction > 0)
        return 0;
    return _layout_param->size()->heightAsFloat() + _line_height;
}

std::vector<Text::LayoutChar> Text::getCharacterMetrics(const GlyphContents& glyphs) const
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
            integral += xScale * std::get<0>(val).aabb().x();
            metrics.emplace_back(std::get<0>(val), integral, std::get<1>(val), std::get<2>(val), isLineBreak);
        }
        else
        {
            int32_t type = static_cast<int32_t>(c);
            const sp<Model> model = _model_loader->loadModel(type);
            const Metrics& m = model->metrics();
            bool iscjk = isCJK(c);
            bool iswordbreak = isWordBreaker(c);
            integral += xScale * m.aabb().x();
            mmap.insert(std::make_pair(c, std::make_tuple(m, iscjk, iswordbreak)));
            metrics.emplace_back(m, integral, iscjk, iswordbreak, isLineBreak);
        }
    }
    return metrics;
}

bool Text::isCJK(int32_t c) const
{
    return c == 0x3005 || Math::between<int32_t>(0x3400, 0x4DBF, c) || Math::between<int32_t>(0x4E00, 0x9FFF, c) ||
           Math::between<int32_t>(0xF900, 0xFAFF, c) || Math::between<int32_t>(0x20000, 0x2A6DF, c) || Math::between<int32_t>(0x2A700, 0x2B73F, c) ||
           Math::between<int32_t>(0x2B740, 0x2B81F, c) || Math::between<int32_t>(0x2F800, 0x2FA1F, c);
}

bool Text::isWordBreaker(wchar_t c) const
{
    return c != '_' && !std::iswalpha(c);
}

Text::GlyphContents Text::makeGlyphs(GlyphMaker& gm, const std::wstring& text)
{
    Text::GlyphContents glyphs = gm.makeGlyphs(text);
    DCHECK(glyphs.size() == text.size(), "Bad GlyphMaker result returned, size mismatch(%d, %d), text: %s", glyphs.size(), text.size(), Strings::toUTF8(text).c_str());
    for(size_t i = 0; i < text.size(); ++i)
        glyphs[i]->setCharacter(text.at(i));
    return glyphs;
}

Text::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _bean_factory(factory), _text(factory.getBuilder<StringVar>(manifest, Constants::Attributes::TEXT)), _render_layer(factory.getBuilder<RenderLayer>(manifest, Constants::Attributes::RENDER_LAYER)),
      _layer_context(sp<LayerContext::BUILDER>::make(factory, manifest, Layer::TYPE_DYNAMIC)), _glyph_maker(factory.getBuilder<GlyphMaker>(manifest, "glyph-maker")),
      _text_scale(factory.getBuilder<String>(manifest, "text-scale")), _letter_spacing(factory.getBuilder<Numeric>(manifest, "letter-spacing")),
      _line_height(Documents::getAttribute<float>(manifest, "line-height", 0.0f)), _line_indent(Documents::getAttribute<float>(manifest, "line-indent", 0.0f))
{
}

sp<Text> Text::BUILDER::build(const Scope& args)
{
    float textScale = _text_scale ? Strings::parse<float>(_text_scale->build(args)) : 1.0f;
    sp<RenderLayer> renderLayer = _render_layer->build(args);
    if(renderLayer)
        return sp<Text>::make(_bean_factory, std::move(renderLayer), _layer_context->build(args), _text->build(args), _glyph_maker->build(args), textScale, BeanUtils::toFloat(_letter_spacing, args, 0.0f), _line_height, _line_indent);
    return sp<Text>::make(_bean_factory, nullptr, _layer_context->build(args), _text->build(args), _glyph_maker->build(args), textScale, BeanUtils::toFloat(_letter_spacing, args, 0.0f), _line_height, _line_indent);
}

Text::LayoutChar::LayoutChar(const Metrics& metrics, float widthIntegral, bool isCJK, bool isWordBreak, bool isLineBreak)
    : _metrics(metrics), _width_integral(widthIntegral), _is_cjk(isCJK), _is_word_break(isWordBreak), _is_line_break(isLineBreak)
{
}

Text::Content::Content()
    : _needs_reload(false)
{
}

bool Text::Content::preSnapshot(const RenderRequest& renderRequest, LayerContext& /*lc*/)
{
    bool needsReload = _position.update(renderRequest.timestamp()) || _needs_reload;
    for(size_t i = 0; i < _render_objects.size(); ++i)
    {
        const sp<RenderObject>& renderObject = _render_objects.at(i);
        if(renderObject)
        {
            Renderable::State state = renderObject->updateState(renderRequest);
            if(state == Renderable::RENDERABLE_STATE_DISPOSED)
                _render_objects[i] = nullptr;
            _render_object_states[i] = state;
        }
    }
    _needs_reload = false;
    return needsReload;
}

void Text::Content::snapshot(const RenderRequest& renderRequest, const LayerContext& lc, RenderLayerSnapshot& output)
{
    const PipelineInput& pipelineInput = output.pipelineInput();
    for(size_t i = 0; i < _render_objects.size(); ++i)
    {
        const sp<RenderObject>& renderObject = _render_objects.at(i);
        if(renderObject)
        {
            Renderable::State state = _render_object_states.at(i);
            if(output.needsReload())
                Renderable::setState(state, Renderable::RENDERABLE_STATE_DIRTY, true);
            output.addSnapshot(lc, renderObject->snapshot(pipelineInput, renderRequest, lc._position + _position.val(), state));
        }
    }
}

SafeVar<Vec3>& Text::Content::position()
{
    return _position;
}

const std::vector<sp<RenderObject>>& Text::Content::renderObjects() const
{
    return _render_objects;
}

void Text::Content::setRenderObjects(std::vector<sp<RenderObject>> renderObjects)
{
    _needs_reload = true;
    _render_objects = std::move(renderObjects);
    _render_object_states = std::vector<Renderable::State>(_render_objects.size());
}

}
