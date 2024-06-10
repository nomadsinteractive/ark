#include "graphics/base/text.h"

#include <cwctype>

#include "core/ark.h"
#include "core/base/resource_loader.h"
#include "core/inf/array.h"
#include "core/inf/variable.h"
#include "core/types/global.h"
#include "core/util/bean_utils.h"
#include "core/util/math.h"
#include "core/util/string_type.h"

#include "graphics/base/layer_context.h"
#include "graphics/base/glyph.h"
#include "graphics/base/render_layer.h"
#include "graphics/base/render_object.h"
#include "graphics/base/render_request.h"
#include "graphics/base/size.h"
#include "graphics/base/v3.h"
#include "graphics/impl/glyph_maker/glyph_maker_span.h"
#include "graphics/inf/glyph_maker.h"
#include "graphics/inf/layout.h"

#include "renderer/base/atlas.h"
#include "renderer/base/model.h"
#include "renderer/base/render_engine.h"
#include "renderer/base/resource_loader_context.h"
#include "renderer/inf/model_loader.h"

#include "app/base/application_context.h"
#include "graphics/traits/layout_param.h"

namespace ark {

namespace {

typedef std::vector<sp<Glyph>> GlyphContents;

struct Character {
    sp<Glyph> _glyph;
    sp<Model> _model;
    V2 _offset;
    float _width_integral;
    bool _is_cjk;
    bool _is_word_break;
    bool _is_line_break;
};

bool isCJK(int32_t c)
{
    return c == 0x3005 || Math::between<int32_t>(0x3400, 0x4DBF, c) || Math::between<int32_t>(0x4E00, 0x9FFF, c) ||
           Math::between<int32_t>(0xF900, 0xFAFF, c) || Math::between<int32_t>(0x20000, 0x2A6DF, c) || Math::between<int32_t>(0x2A700, 0x2B73F, c) ||
           Math::between<int32_t>(0x2B740, 0x2B81F, c) || Math::between<int32_t>(0x2F800, 0x2FA1F, c);
}

bool isWordBreaker(wchar_t c)
{
    return c != '_' && !std::iswalpha(c);
}

V2 getCharacterOffset(const Model& model)
{
    const Boundaries& bounds = model.content();
    const Boundaries& occupies = model.occupy();
    const V2 bitmapContentSize = bounds.size()->val();
    const V2 bitmapOccupySize = occupies.size()->val();
    const V2 bitmapPos = -occupies.aabbMin()->val();
    return V2(bitmapPos.x(), bitmapOccupySize.y() - bitmapPos.y() - bitmapContentSize.y());
}

std::vector<Character> toLayoutCharacters(const GlyphContents& glyphs, float letterScale, ModelLoader& modelLoader)
{
    std::unordered_map<wchar_t, std::tuple<sp<Model>, V2, bool, bool>> mmap;
    const float xScale = letterScale;
    float integral = 0;
    std::vector<Character> layoutChars;
    layoutChars.reserve(glyphs.size());
    for(const sp<Glyph>& i : glyphs)
    {
        const wchar_t c = i->character();
        const bool isLineBreak = c == '\n';
        if(const auto iter = mmap.find(c); iter != mmap.end())
        {
            const auto& [model, offset, iscjk, iswordbreak] = iter->second;
            const V3& occupy = model->occupy()->size()->val();
            integral += xScale * occupy.x();
            layoutChars.push_back({i, model, offset, integral, iscjk, iswordbreak, isLineBreak});
        }
        else
        {
            int32_t type = static_cast<int32_t>(c);
            sp<Model> model = modelLoader.loadModel(type);
            const V2 offset = getCharacterOffset(model) * letterScale;
            const Boundaries& m = model->occupy();
            bool iscjk = isCJK(c);
            bool iswordbreak = isWordBreaker(c);
            integral += xScale * m.size()->val().x();
            mmap.insert(std::make_pair(c, std::make_tuple(model, offset, iscjk, iswordbreak)));
            layoutChars.push_back({i, std::move(model), offset, integral, iscjk, iswordbreak, isLineBreak});
        }
    }
    return layoutChars;
}

GlyphContents makeGlyphs(GlyphMaker& gm, const std::wstring& text)
{
    GlyphContents glyphs = gm.makeGlyphs(text);
    CHECK(glyphs.size() == text.size(), "Bad GlyphMaker result returned, size mismatch(%d, %d), text: %s", glyphs.size(), text.size(), Strings::toUTF8(text).c_str());
    for(size_t i = 0; i < text.size(); ++i)
        glyphs.at(i)->setCharacter(text.at(i));
    return glyphs;
}

void doFlowLayout(std::vector<Layout::Hierarchy>& childNodes, float letterSpacing, float flowX, float flowY)
{
    for(const Layout::Hierarchy& i : childNodes) {
        Layout::Node& node = i._node;
        node.setOffsetPosition(V2(flowX, flowY));
        flowX += letterSpacing + node.size()->x();
    }
}

struct RenderableCharacter : Renderable {
    RenderableCharacter(sp<Renderable> delegate, sp<Layout::Node> layoutNode, const V2& letterOffset)
        : _delegate(std::move(delegate)), _layout_node(std::move(layoutNode)), _letter_offset(letterOffset) {
    }

    StateBits updateState(const RenderRequest& renderRequest) override {
        StateBits stateBits = _delegate->updateState(renderRequest);
        if(_layout_node->update(renderRequest.timestamp()))
            stateBits = static_cast<StateBits>(stateBits | RENDERABLE_STATE_DIRTY);
        return stateBits;
    }

    Snapshot snapshot(const LayerContextSnapshot& snapshotContext, const RenderRequest& renderRequest, StateBits state) override {
        Snapshot snapshot = _delegate->snapshot(snapshotContext, renderRequest, state);
        snapshot._position += V3(_layout_node->offsetPosition() + _letter_offset, 0);
        return snapshot;
    }

    sp<Renderable> _delegate;
    sp<Layout::Node> _layout_node;
    V2 _letter_offset;
};

struct UpdatableFlexStart : Updatable {
    UpdatableFlexStart(Layout::Hierarchy hierarchy, float letterSpacing)
        : _hierarchy((std::move(hierarchy))), _letter_spacing(letterSpacing) {
    }

    bool update(uint64_t timestamp) override {
        doFlowLayout(_hierarchy._child_nodes, _letter_spacing, 0, 0);
        return false;
    }

    Layout::Hierarchy _hierarchy;
    float _letter_spacing;
};

struct UpdatableCenter : Updatable {
    UpdatableCenter(Layout::Hierarchy hierarchy, Size size, float letterSpacing)
        : _hierarchy((std::move(hierarchy))), _letter_spacing(letterSpacing), _size(std::move(size)) {
    }

    bool update(uint64_t timestamp) override {
        const float flowX = (_size.widthAsFloat() - _hierarchy._node->size()->x()) / 2;
        const float flowY = (_size.heightAsFloat() - _hierarchy._node->size()->y()) / 2;
        doFlowLayout(_hierarchy._child_nodes, _letter_spacing, flowX, flowY);
        return false;
    }

    Layout::Hierarchy _hierarchy;
    float _letter_spacing;
    Size _size;
};

struct UpdatableFlexEnd : Updatable {
    UpdatableFlexEnd(Layout::Hierarchy hierarchy, Size size, float letterSpacing)
        : _hierarchy((std::move(hierarchy))), _letter_spacing(letterSpacing), _size(std::move(size)) {
    }

    bool update(uint64_t timestamp) override {
        const float flowX = _size.widthAsFloat() - _hierarchy._node->size()->x();
        const float flowY = (_size.heightAsFloat() - _hierarchy._node->size()->y()) / 2;
        doFlowLayout(_hierarchy._child_nodes, _letter_spacing, flowX, flowY);
        return false;
    }

    Layout::Hierarchy _hierarchy;
    float _letter_spacing;
    Size _size;
};

struct LayoutLabel : Layout {
    LayoutLabel(float letterSpacing)
        : _letter_spacing(letterSpacing) {
    }

    sp<Updatable> inflate(Hierarchy hierarchy) override {
        const LayoutParam& lp = hierarchy._node->_layout_param;
        Size size(lp.width()._value.val(), lp.height()._value.val());
        switch(lp.justifyContent()) {
            case LayoutParam::JUSTIFY_CONTENT_CENTER:
                return sp<Updatable>::make<UpdatableCenter>(std::move(hierarchy), std::move(size), _letter_spacing);
            case LayoutParam::JUSTIFY_CONTENT_FLEX_END:
                return sp<Updatable>::make<UpdatableFlexEnd>(std::move(hierarchy), std::move(size), _letter_spacing);
            case LayoutParam::JUSTIFY_CONTENT_FLEX_START:
            default:
                break;
        }
        return sp<Updatable>::make<UpdatableFlexStart>(std::move(hierarchy), _letter_spacing);
    }

    float _letter_spacing;
};

}

struct Text::Content {
    Content(sp<RenderLayer> renderLayer, sp<StringVar> string, sp<LayoutParam> layoutParam, sp<GlyphMaker> glyphMaker, float textScale, float letterSpacing, float lineHeight, float lineIndent)
        : _render_layer(std::move(renderLayer)), _string(string ? std::move(string) : StringType::create()), _layout_param(std::move(layoutParam)), _glyph_maker(glyphMaker ? std::move(glyphMaker) : sp<GlyphMaker>::make<GlyphMakerSpan>()),
          _text_scale(textScale), _letter_spacing(letterSpacing), _layout_direction(Ark::instance().applicationContext()->renderEngine()->toLayoutDirection(1.0f)), _line_height(_layout_direction * lineHeight), _line_indent(lineIndent),
          _size(sp<Size>::make(0.0f, 0.0f)), _position(sp<VariableWrapper<V3>>::make(V3()))
    {
        if(_string->val() && !_string->val()->empty())
            setText(Strings::fromUTF8(*_string->val()));
    }

    bool update(uint64_t timestamp)
    {
        bool contentDirty = _string->update(timestamp);
        if(contentDirty)
            setText(Strings::fromUTF8(*_string->val()));
        bool layoutDirty = _updatable_layout ? _updatable_layout->update(timestamp) : false;
        return contentDirty || layoutDirty;
    }

    void setText(std::wstring text);
    void setRichText(std::wstring richText, const sp<ResourceLoader>& resourceLoader, const Scope& args);

    void createContent();
    void createRichContent(const Scope& args, BeanFactory& factory);

    float doCreateRichContent(GlyphContents& cm, GlyphMaker& gm, const document& richtext, BeanFactory& factory, const Scope& args, float& flowx, float& flowy, float boundary);
    float doLayoutWithBoundary(GlyphContents& cm, float& flowx, float& flowy, float boundary);

    V2 doLayoutWithoutBoundary() const {
        float lineHeight = 0;
        float flowx = _layout_chars.empty() ? 0 : -_letter_spacing;
        for(const Character& i : _layout_chars) {
            const Model& model = i._model;
            const Boundaries& occupies = model.occupy();
            const V2 bitmapOccupySize = V2(occupies.size()->val()) * _text_scale;
            lineHeight = std::max(bitmapOccupySize.y(), lineHeight);
            Glyph& glyph = i._glyph;
            glyph.setOccupySize(bitmapOccupySize);
            flowx += bitmapOccupySize.x() + _letter_spacing;
        }
        return V2(flowx, lineHeight);
    }

    void createLayerContent(const V2& layoutSize) {
        _render_objects.clear();
        for(const sp<Glyph>& i : _glyphs)
            _render_objects.push_back(i->toRenderObject());

        _size->setWidth(layoutSize.x());
        _size->setHeight(layoutSize.y());

        updateLayoutContent();
    }

    void updateLayoutContent()
    {
        if(_layer_context)
            _layer_context->clear();
        else
            _layer_context = _render_layer->makeLayerContext(nullptr, _position, nullptr, nullptr);

        Layout::Hierarchy hierarchy = makeHierarchy();
        DASSERT(_render_objects.size() == hierarchy._child_nodes.size());
        for(size_t i = 0; i < _render_objects.size(); ++i)
            _layer_context->add(sp<RenderableCharacter>::make(_render_objects.at(i), hierarchy._child_nodes.at(i)._node, _layout_chars.at(i)._offset));

        const sp<Layout>& layout = _layout_param && _layout_param->layout() ? _layout_param->layout() : sp<Layout>::make<LayoutLabel>(_letter_spacing);
        _updatable_layout = layout->inflate(std::move(hierarchy));
    }

    void nextLine(float fontHeight, float& flowx, float& flowy) const
    {
        flowy += (_line_height != 0 ? _line_height : (fontHeight * _layout_direction));
        flowx = _line_indent;
    }

    float getFlowY() const;
    float getLayoutBoundary() const;

    Layout::Hierarchy makeHierarchy() {
        Layout::Hierarchy hierarchy{sp<Layout::Node>::make(_layout_param)};
        hierarchy._node->setSize(_size->val());
        for(Character& i : _layout_chars) {
            sp<Layout::Node> node = sp<Layout::Node>::make(sp<LayoutParam>::make(i._glyph->occupySize().x(), i._glyph->occupySize().y()), &i);
            hierarchy._child_nodes.push_back({std::move(node)});
        }
        return hierarchy;
    }

    sp<RenderLayer> _render_layer;
    sp<StringVar> _string;
    sp<LayoutParam> _layout_param;
    sp<LayerContext> _layer_context;
    sp<GlyphMaker> _glyph_maker;
    sp<Updatable> _updatable_layout;

    float _text_scale;
    float _letter_spacing;
    float _layout_direction;
    float _line_height;
    float _line_indent;

    sp<Size> _size;
    sp<Boundaries> _boundaries;

    std::wstring _text_unicode;
    std::vector<sp<Glyph>> _glyphs;

    sp<VariableWrapper<V3>> _position;
    std::vector<sp<RenderObject>> _render_objects;
    std::vector<Character> _layout_chars;
};

class RenderBatchContent : public RenderBatch {
public:
    RenderBatchContent(sp<Text::Content> content, sp<Boolean> discarded)
        : RenderBatch(std::move(discarded)), _content(std::move(content)) {
    }

    std::vector<sp<LayerContext>>& snapshot(const RenderRequest& renderRequest) override {
        _layer_contexts.clear();
        _content->update(renderRequest.timestamp());
        _layer_contexts.push_back(_content->_layer_context);
        return _layer_contexts;
    }

private:
    sp<Text::Content> _content;
    std::vector<sp<LayerContext>> _layer_contexts;
};

Text::Text(sp<RenderLayer> renderLayer, sp<StringVar> content, sp<LayoutParam> layoutParam, sp<GlyphMaker> glyphMaker, float textScale, float letterSpacing, float lineHeight, float lineIndent)
    : _render_layer(std::move(renderLayer)), _content(sp<Content>::make(_render_layer, std::move(content), std::move(layoutParam), std::move(glyphMaker), textScale, letterSpacing, lineHeight, lineIndent))
{
}

const std::vector<sp<RenderObject>>& Text::contents() const
{
    return _content->_render_objects;
}

const sp<LayoutParam>& Text::layoutParam() const
{
    return _content->_layout_param;
}

void Text::setLayoutParam(sp<LayoutParam> layoutParam)
{
    _content->_layout_param = std::move(layoutParam);
    _content->updateLayoutContent();
}

sp<Vec3> Text::position() const
{
    return _content->_position;
}

void Text::setPosition(sp<Vec3> position)
{
    _content->_position->reset(std::move(position));
}

const sp<Size>& Text::size() const
{
    return _content->_size;
}

const sp<Boundaries>& Text::boundaries() const
{
    return _content->_boundaries;
}

void Text::setBoundaries(sp<Boundaries> boundaries)
{
    _content->_boundaries = std::move(boundaries);
}

const std::wstring& Text::text() const
{
    return _content->_text_unicode;
}

void Text::setText(std::wstring text)
{
    _content->setText(std::move(text));
}

void Text::show(sp<Boolean> discarded)
{
    hide();

    _render_batch = sp<RenderBatchContent>::make(_content, std::move(discarded));
    _render_layer->addRenderBatch(_render_batch);
}

void Text::hide()
{
    if(_render_batch)
        _render_batch->setDiscarded(Global<Constants>()->BOOLEAN_TRUE);
    _render_batch = nullptr;
}

void Text::setRichText(std::wstring richText, const sp<ResourceLoader>& resourceLoader, const Scope& args)
{
    _content->setRichText(std::move(richText), resourceLoader, args);
}

bool Text::update(uint64_t timestamp) const
{
    return _content->update(timestamp);
}

void Text::Content::setText(std::wstring text)
{
    _text_unicode = std::move(text);
    createContent();
}

void Text::Content::setRichText(std::wstring richText, const sp<ResourceLoader>& resourceLoader, const Scope& args)
{
    _text_unicode = std::move(richText);
    createRichContent(args, resourceLoader ? resourceLoader->beanFactory() : Ark::instance().applicationContext()->resourceLoader()->beanFactory());
}

void Text::Content::createContent()
{
    _glyphs = makeGlyphs(_glyph_maker, _text_unicode);
    _layout_chars = toLayoutCharacters(_glyphs, _text_scale, _render_layer->context()->modelLoader());
    createLayerContent(doLayoutWithoutBoundary());
}

void Text::Content::createRichContent(const Scope& args, BeanFactory& factory)
{
    float boundary = getLayoutBoundary();
    float flowx = boundary > 0 ? 0 : -_letter_spacing, flowy = getFlowY();
    const document richtext = Documents::parseFull(Strings::toUTF8(_text_unicode));
    _glyphs.clear();
    float height = doCreateRichContent(_glyphs, _glyph_maker, richtext, factory, args, flowx, flowy, boundary);
    createLayerContent(V2(flowx, height));
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

// float Text::Content::doLayoutWithBoundary(GlyphContents& cm, float& flowx, float& flowy, float boundary)
// {
//     _layout_chars = toLayoutCharacters(cm, _text_scale, _render_layer->context()->modelLoader());
//
//     const float fontHeight = _layout_chars.size() > 0 ? _layout_chars.at(0)._model->occupy()->size()->val().y() * _text_scale : 0;
//     size_t begin = 0;
//     for(size_t i = 0; i < _layout_chars.size(); ++i)
//     {
//         size_t end = i + 1;
//         const LayoutChar& currentChar = _layout_chars.at(i);
//         bool allowLineBreak = currentChar._is_cjk || currentChar._is_word_break;
//         if(end == _layout_chars.size() || allowLineBreak)
//         {
//             float beginWidth = begin > 0 ? _layout_chars.at(begin - 1)._width_integral : 0;
//             float placingWidth = currentChar._width_integral - beginWidth;
//             if(flowx + placingWidth > boundary && allowLineBreak)
//             {
//                 if(flowx != _line_indent)
//                     nextLine(fontHeight, flowx, flowy);
//                 else
//                     LOGW("No other choices, placing word out of boundary(%.2f)", boundary);
//             }
//
//             if(end - begin == 1)
//                 placeOne(_text_scale, currentChar, flowx, flowy);
//             else
//                 place(_layout_chars, _letter_spacing, _text_scale, begin, end, flowx, flowy);
//
//             begin = i + 1;
//         }
//     }
//
//     return std::abs(flowy) + fontHeight;
// }

float Text::Content::getFlowY() const
{
    if(!_boundaries || _layout_direction > 0)
        return 0;
    return _boundaries->size()->val().y() + _line_height;
}

float Text::Content::getLayoutBoundary() const
{
    return _boundaries ? _boundaries->size()->val().x() : 0;
}

Text::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _render_layer(factory.ensureBuilder<RenderLayer>(manifest, constants::RENDER_LAYER)), _text(factory.getBuilder<StringVar>(manifest, constants::TEXT)), _layout_param(factory.getConcreteClassBuilder<LayoutParam>(manifest, constants::LAYOUT_PARAM)),
      _glyph_maker(factory.getBuilder<GlyphMaker>(manifest, "glyph-maker")), _text_scale(factory.getBuilder<String>(manifest, "text-scale")), _letter_spacing(factory.getBuilder<Numeric>(manifest, "letter-spacing")),
      _line_height(Documents::getAttribute<float>(manifest, "line-height", 0.0f)), _line_indent(Documents::getAttribute<float>(manifest, "line-indent", 0.0f))
{
}

sp<Text> Text::BUILDER::build(const Scope& args)
{
    float textScale = _text_scale ? Strings::eval<float>(_text_scale->build(args)) : 1.0f;
    float letterSpacing = _letter_spacing ? _letter_spacing->build(args)->val() : 0.0f;
    return sp<Text>::make(_render_layer->build(args), _text->build(args), _layout_param->build(args), _glyph_maker->build(args), textScale, letterSpacing, _line_height, _line_indent);
}

}
