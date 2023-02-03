#include "graphics/base/text.h"

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
    : Text(nullptr, layer, std::move(text), nullptr, textScale, letterSpacing, lineHeight, lineIndent)
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

Text::Text(sp<RenderLayer> renderLayer, sp<LayerContext> layerContext, sp<StringVar> text, sp<GlyphMaker> glyphMaker, float textScale, float letterSpacing, float lineHeight, float lineIndent)
    : _render_layer(std::move(renderLayer)), _layer_context(std::move(layerContext)), _content(sp<Content>::make(_layer_context, std::move(text), std::move(glyphMaker), _layer_context->modelLoader(), textScale,
                                                                                                                 letterSpacing, lineHeight, lineIndent))
{
    WARN(_render_layer, "Text without a RenderLayer will no longer be supported");
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
    _content->_needs_reload = true;
}

const sp<Size>& Text::size() const
{
    return _content->_size;
}

const sp<Size>& Text::layoutSize() const
{
    return _content->_layout_size;
}

void Text::setLayoutSize(sp<Size> layoutSize)
{
    _content->_layout_size = std::move(layoutSize);
    _content->layoutContent();
}

const std::wstring& Text::text() const
{
    return _content->_text_unicode;
}

void Text::setText(std::wstring text)
{
    _content->setText(_layer_context, std::move(text));
}

void Text::show(sp<Boolean> disposed)
{
    _layer_context = _render_layer->makeLayerContext(_content, nullptr, nullptr, std::move(disposed));
}

void Text::setRichText(std::wstring richText, const sp<ResourceLoader>& resourceLoader, const Scope& args)
{
    _content->setRichText(_layer_context, std::move(richText), resourceLoader, args);
}

void Text::render(RenderRequest& renderRequest, const V3& position)
{
    return;

    if(!_content->update(_layer_context, renderRequest.timestamp()))
        if(_content->_layout_size && _content->_layout_size->update(renderRequest.timestamp()))
            _content->layoutContent();

    _layer_context->renderRequest(V3());
    for(const sp<RenderablePassive>& i : _content->_renderables)
        i->requestUpdate(position);
}

bool Text::update(uint64_t timestamp) const
{
    return _content->update(_layer_context, timestamp);
}

bool Text::isCJK(int32_t c)
{
    return c == 0x3005 || Math::between<int32_t>(0x3400, 0x4DBF, c) || Math::between<int32_t>(0x4E00, 0x9FFF, c) ||
           Math::between<int32_t>(0xF900, 0xFAFF, c) || Math::between<int32_t>(0x20000, 0x2A6DF, c) || Math::between<int32_t>(0x2A700, 0x2B73F, c) ||
           Math::between<int32_t>(0x2B740, 0x2B81F, c) || Math::between<int32_t>(0x2F800, 0x2FA1F, c);
}

bool Text::isWordBreaker(wchar_t c)
{
    return c != '_' && !std::iswalpha(c);
}

Text::GlyphContents Text::makeGlyphs(GlyphMaker& gm, const std::wstring& text)
{
    Text::GlyphContents glyphs = gm.makeGlyphs(text);
    CHECK(glyphs.size() == text.size(), "Bad GlyphMaker result returned, size mismatch(%d, %d), text: %s", glyphs.size(), text.size(), Strings::toUTF8(text).c_str());
    for(size_t i = 0; i < text.size(); ++i)
        glyphs.at(i)->setCharacter(text.at(i));
    return glyphs;
}

void Text::Content::setText(const sp<LayerContext>& layerContext, std::wstring text)
{
    _text_unicode = std::move(text);
    createContent(layerContext);
}

void Text::Content::setRichText(const sp<LayerContext>& layerContext, std::wstring richText, const sp<ResourceLoader>& resourceLoader, const Scope& args)
{
    _text_unicode = std::move(richText);
    createRichContent(layerContext, args, resourceLoader ? resourceLoader->beanFactory() : Ark::instance().applicationContext()->resourceLoader()->beanFactory());
}

bool Text::Content::update(const sp<LayerContext>& layerContext, uint64_t timestamp)
{
    bool positionDirty = _position.update(timestamp);
    bool contentDirty = _string->update(timestamp);
    bool sizeDirty = _layout_size && _layout_size->update(timestamp);
    if(contentDirty)
        setText(layerContext, Strings::fromUTF8(*_string->val()));
    if(sizeDirty)
        layoutContent();
    return positionDirty || contentDirty || sizeDirty;
}

void Text::Content::createContent(const sp<LayerContext>& layerContext)
{
    float boundary = getLayoutBoundary();
    float flowx = boundary > 0 ? 0 : -_letter_spacing, flowy = getFlowY();
    _glyphs = makeGlyphs(_glyph_maker, _text_unicode);
    flowy = doLayoutContent(_glyphs, flowx, flowy, boundary);
    createLayerContent(layerContext, flowx, flowy);
}

float Text::Content::doLayoutContent(GlyphContents& cm, float& flowx, float& flowy, float boundary)
{
    return boundary > 0 ? doLayoutWithBoundary(cm, flowx, flowy, boundary) : doLayoutWithoutBoundary(cm, flowx, flowy);
}

void Text::Content::createRichContent(const sp<LayerContext>& layerContext, const Scope& args, BeanFactory& factory)
{
    float boundary = getLayoutBoundary();
    float flowx = boundary > 0 ? 0 : -_letter_spacing, flowy = getFlowY();
    const document richtext = Documents::parseFull(Strings::toUTF8(_text_unicode));
    _glyphs.clear();
    float height = doCreateRichContent(_glyphs, _glyph_maker, richtext, factory, args, flowx, flowy, boundary);
    createLayerContent(layerContext, flowx, height);
}

float Text::Content::doCreateRichContent(GlyphContents& cm, GlyphMaker& gm, const document& richtext, BeanFactory& factory, const Scope& args, float& flowx, float& flowy, float boundary)
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

void Text::Content::layoutContent()
{
    DCHECK(_render_objects.size() == _text_unicode.length(), "Contents have changed, cannot do relayout");
    float boundary = getLayoutBoundary();
    float flowx = boundary > 0 ? 0 : -_letter_spacing, flowy = getFlowY();
    flowy = doLayoutContent(_glyphs, flowx, flowy, boundary);
    for(size_t i = 0; i < _render_objects.size(); ++i)
        _render_objects.at(i)->setPosition(_glyphs.at(i)->toRenderObjectPosition());
    _size->setWidth(flowx);
    _size->setHeight(flowy);
}

void Text::Content::createLayerContent(const sp<LayerContext>& layerContext, float width, float height)
{
    std::vector<sp<RenderObject>> renderObjects;

    for(const sp<Glyph>& i : _glyphs)
        renderObjects.push_back(i->toRenderObject());

    _size->setWidth(width);
    _size->setHeight(height);

    _renderables.clear();
    if(layerContext && false)
        for(const sp<RenderObject>& i : renderObjects)
        {
            sp<RenderablePassive> renderable = sp<RenderablePassive>::make(i);
            layerContext->add(renderable, nullptr, sp<BooleanByWeakRef<RenderablePassive>>::make(renderable, 1));
            _renderables.push_back(std::move(renderable));
        }

    setRenderObjects(std::move(renderObjects));
}

float Text::Content::doLayoutWithBoundary(GlyphContents& cm, float& flowx, float& flowy, float boundary)
{
    const std::vector<LayoutChar> layoutChars = toLayoutCharacters(cm);
    float fontHeight = layoutChars.size() > 0 ? layoutChars.at(0)._model->occupies()->size().y() * _text_scale : 0;
    size_t begin = 0;
    for(size_t i = 0; i < layoutChars.size(); ++i)
    {
        size_t end = i + 1;
        const LayoutChar& currentChar = layoutChars.at(i);
        bool allowLineBreak = currentChar._is_cjk || currentChar._is_word_break;
        if(end == layoutChars.size() || allowLineBreak)
        {
            float beginWidth = begin > 0 ? layoutChars.at(begin - 1)._width_integral : 0;
            float placingWidth = currentChar._width_integral - beginWidth;
            if(flowx + placingWidth > boundary && allowLineBreak)
            {
                if(flowx != _line_indent)
                    nextLine(fontHeight, flowx, flowy);
                else
                    LOGW("No other choices, placing word out of boundary(%.2f)", boundary);
            }

            if(end - begin == 1)
                placeOne(cm[i], currentChar._model, flowx, flowy);
            else
                place(cm, layoutChars, begin, end, flowx, flowy);

            begin = i + 1;
        }
    }

    return std::abs(flowy) + fontHeight;
}

float Text::Content::doLayoutWithoutBoundary(GlyphContents& cm, float& flowx, float flowy)
{
    float fontHeight = 0;
    for(Glyph& i : cm)
    {
        const sp<Model> model = _model_loader->loadModel(i.type()->val());
        flowx += _letter_spacing;
        placeOne(i, model, flowx, flowy, &fontHeight);
    }
    return std::abs(flowy) + fontHeight;
}

void Text::Content::place(GlyphContents& cm, const std::vector<Text::LayoutChar>& layouts, size_t begin, size_t end, float& flowx, float flowy)
{
    for(size_t i = begin; i < end; ++i)
    {
        if(begin > 0)
            flowx += _letter_spacing;

        const Text::LayoutChar& layoutChar = layouts.at(i);
        placeOne(cm[i], layoutChar._model, flowx, flowy);
    }
}

void Text::Content::placeOne(Glyph& glyph, const Model& model, float& flowx, float flowy, float* fontHeight)
{
    const V2 scale = V2(_text_scale, _text_scale);
    const Metrics& bounds = model.bounds();
    const Metrics& occupies = model.occupies();
    float bitmapWidth = scale.x() * bounds.width();
    float bitmapHeight = scale.y() * bounds.height();
    float width = scale.x() * occupies.width();
    float height = scale.y() * occupies.height();
    float bitmapX = - scale.x() * occupies.aabbMin().x();
    float bitmapY = - scale.y() * occupies.aabbMin().y();
    if(fontHeight)
        *fontHeight = std::max(height, *fontHeight);
    glyph.setLayoutPosition(V3(flowx + bitmapX, flowy + height - bitmapY - bitmapHeight, 0));
    glyph.setLayoutSize(V2(bitmapWidth, bitmapHeight));
    flowx += width;
}

void Text::Content::nextLine(float fontHeight, float& flowx, float& flowy) const
{
    flowy += (_line_height != 0 ? _line_height : (fontHeight * _layout_direction));
    flowx = _line_indent;
}

float Text::Content::getFlowY() const
{
    if(!_layout_size || _layout_direction > 0)
        return 0;
    return _layout_size->heightAsFloat() + _line_height;
}

std::vector<Text::LayoutChar> Text::Content::toLayoutCharacters(const GlyphContents& glyphs) const
{
    std::vector<LayoutChar> layoutChars;
    std::unordered_map<wchar_t, std::tuple<sp<Model>, bool, bool>> mmap;
    const float xScale = _text_scale;
    float integral = 0;
    layoutChars.reserve(glyphs.size());
    for(const sp<Glyph>& i : glyphs)
    {
        const wchar_t c = i->character();
        const bool isLineBreak = c == '\n';
        const auto iter = mmap.find(c);
        if(iter != mmap.end())
        {
            const std::tuple<sp<Model>, bool, bool>& val = iter->second;
            integral += xScale * std::get<0>(val)->occupies()->width();
            layoutChars.emplace_back(std::get<0>(val), integral, std::get<1>(val), std::get<2>(val), isLineBreak);
        }
        else
        {
            int32_t type = static_cast<int32_t>(c);
            sp<Model> model = _model_loader->loadModel(type);
            const Metrics& m = model->occupies();
            bool iscjk = isCJK(c);
            bool iswordbreak = isWordBreaker(c);
            integral += xScale * m.size().x();
            mmap.insert(std::make_pair(c, std::make_tuple(model, iscjk, iswordbreak)));
            layoutChars.emplace_back(std::move(model), integral, iscjk, iswordbreak, isLineBreak);
        }
    }
    return layoutChars;
}

float Text::Content::getLayoutBoundary() const
{
    return _layout_size ? _layout_size->widthAsFloat() : 0;
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
    return sp<Text>::make(_render_layer->build(args), _layer_context->build(args), _text->build(args), _glyph_maker->build(args), textScale,
                          BeanUtils::toFloat(_letter_spacing, args, 0.0f), _line_height, _line_indent);
}

Text::LayoutChar::LayoutChar(sp<Model> model, float widthIntegral, bool isCJK, bool isWordBreak, bool isLineBreak)
    : _model(std::move(model)), _width_integral(widthIntegral), _is_cjk(isCJK), _is_word_break(isWordBreak), _is_line_break(isLineBreak)
{
}

Text::Content::Content(const sp<LayerContext>& layerContext, sp<StringVar> string, sp<GlyphMaker> glyphMaker, sp<ModelLoader> modelLoader, float textScale, float letterSpacing, float lineHeight, float lineIndent)
    : _string(string ? std::move(string) : StringType::create()), _glyph_maker(glyphMaker ? std::move(glyphMaker) : sp<GlyphMaker>::make<GlyphMakerSpan>()), _model_loader(std::move(modelLoader)),
      _text_scale(textScale), _letter_spacing(letterSpacing), _layout_direction(Ark::instance().applicationContext()->renderEngine()->toLayoutDirection(1.0f)), _line_height(_layout_direction * lineHeight),
      _line_indent(lineIndent), _size(sp<Size>::make(0.0f, 0.0f)), _needs_reload(false)
{
    if(_string->val() && !_string->val()->empty())
        setText(layerContext, Strings::fromUTF8(*_string->val()));
}

bool Text::Content::preSnapshot(const RenderRequest& renderRequest, LayerContext& /*lc*/)
{
    bool needsReload = update(nullptr, renderRequest.timestamp()) || _needs_reload;
    for(size_t i = 0; i < _render_objects.size(); ++i)
    {
        const sp<RenderObject>& renderObject = _render_objects.at(i);
        if(renderObject)
        {
            Renderable::StateBits state = renderObject->updateState(renderRequest);
            if(state & Renderable::RENDERABLE_STATE_DISPOSED)
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
                state.setState(Renderable::RENDERABLE_STATE_DIRTY, true);
            output.addSnapshot(lc, renderObject->snapshot(pipelineInput, renderRequest, lc._position + _position.val(), state.stateBits()));
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
    _render_object_states = std::vector<Renderable::StateBits>(_render_objects.size());
}

}
