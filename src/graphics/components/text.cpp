#include "graphics/components/text.h"

#include <cwctype>

#include "core/ark.h"
#include "core/base/resource_loader.h"
#include "core/components/discarded.h"
#include "core/inf/variable.h"
#include "core/types/global.h"
#include "core/util/log.h"
#include "core/util/math.h"
#include "core/util/string_type.h"
#include "core/util/updatable_util.h"

#include "graphics/base/font.h"
#include "graphics/base/glyph.h"
#include "graphics/base/layer_context.h"
#include "graphics/base/render_layer.h"
#include "graphics/base/render_request.h"
#include "graphics/base/v3.h"
#include "graphics/components/layout_param.h"
#include "graphics/components/render_object.h"
#include "graphics/components/size.h"
#include "graphics/components/translation.h"
#include "graphics/impl/glyph_maker/glyph_maker_span.h"
#include "graphics/impl/renderable/renderable_with_transform.h"
#include "graphics/inf/glyph_maker.h"
#include "graphics/inf/layout.h"
#include "graphics/util/mat4_type.h"
#include "graphics/util/vec3_type.h"

#include "renderer/base/atlas.h"
#include "renderer/base/model.h"
#include "renderer/base/render_engine.h"
#include "renderer/base/resource_loader_context.h"
#include "renderer/inf/model_loader.h"

#include "app/base/application_context.h"
#include "app/view/view.h"

namespace ark {

namespace {

typedef Vector<sp<Glyph>> GlyphContents;

struct Character {
    sp<Glyph> _glyph;
    sp<Model> _model;
    V2 _offset;
    float _width_integral;
    bool _is_cjk;
    bool _is_word_break;
    bool _is_line_break;
};

bool isCJK(const int32_t c)
{
    return c == 0x3005 || Math::between<int32_t>(0x3400, 0x4DBF, c) || Math::between<int32_t>(0x4E00, 0x9FFF, c) ||
           Math::between<int32_t>(0xF900, 0xFAFF, c) || Math::between<int32_t>(0x20000, 0x2A6DF, c) || Math::between<int32_t>(0x2A700, 0x2B73F, c) ||
           Math::between<int32_t>(0x2B740, 0x2B81F, c) || Math::between<int32_t>(0x2F800, 0x2FA1F, c);
}

bool isWordBreaker(const wchar_t c)
{
    return c != '_' && !std::iswalpha(c);
}

V2 getCharacterOffset(const Model& model)
{
    const Boundaries& content = model.content();
    const Boundaries& occupy = model.occupy();
    const V2 bitmapContentSize = content.size()->val();
    const V2 bitmapOccupySize = occupy.size()->val();
    const V2 bitmapPos = -occupy.aabbMin()->val();
    return {0, 0};
    return {bitmapPos.x(), bitmapOccupySize.y() - bitmapPos.y() - bitmapContentSize.y()};
}

Vector<Character> toLayoutCharacters(const GlyphContents& glyphs, ModelLoader& modelLoader)
{
    HashMap<wchar_t, std::tuple<sp<Model>, V2, bool, bool>> mmap;
    float integral = 0;
    Vector<Character> layoutChars;
    layoutChars.reserve(glyphs.size());
    for(const sp<Glyph>& i : glyphs)
    {
        const wchar_t c = i->character();
        const bool isLineBreak = c == '\n';
        if(const auto iter = mmap.find(c); iter != mmap.end())
        {
            const auto& [model, offset, iscjk, iswordbreak] = iter->second;
            const V3& occupy = model->occupy()->size()->val();
            integral += occupy.x();
            layoutChars.push_back({i, model, offset, integral, iscjk, iswordbreak, isLineBreak});
        }
        else
        {
            const int32_t type = c;
            sp<Model> model = modelLoader.loadModel(i->font() ? i->font()->combine(type) : type);
            const V2 offset = getCharacterOffset(model);
            const Boundaries& m = model->occupy();
            bool iscjk = isCJK(c);
            bool iswordbreak = isWordBreaker(c);
            integral += m.size()->val().x();
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

void doFlowLayout(const Vector<Layout::Hierarchy>& childNodes, const float letterSpacing, float flowX, const float flowY)
{
    for(const Layout::Hierarchy& i : childNodes) {
        Layout::Node& node = i._node;
        node.setOffsetPosition(V2(flowX, flowY));
        flowX += letterSpacing + node.size()->x();
    }
}

class RenderableCharacter final : public Renderable {
public:
    RenderableCharacter(sp<Renderable> delegate, sp<Layout::Node> layoutNode, const V2 offsetPosition)
        : _delegate(std::move(delegate)), _layout_node(std::move(layoutNode)), _offset_position(offsetPosition) {
    }

    StateBits updateState(const RenderRequest& renderRequest) override {
        StateBits stateBits = _delegate->updateState(renderRequest);
        if(_layout_node->update(renderRequest.timestamp()))
            stateBits = static_cast<StateBits>(stateBits | RENDERABLE_STATE_DIRTY);
        return stateBits;
    }

    Snapshot snapshot(const LayerContextSnapshot& snapshotContext, const RenderRequest& renderRequest, StateBits state) override {
        Snapshot snapshot = _delegate->snapshot(snapshotContext, renderRequest, state);
        snapshot._position += V3((_layout_node->offsetPosition() + _offset_position), 0);
        snapshot._size = snapshot._size;
        return snapshot;
    }

private:
    sp<Renderable> _delegate;
    sp<Layout::Node> _layout_node;
    V2 _offset_position;
};

class UpdatableFlexStart final : public Updatable {
public:
    UpdatableFlexStart(Layout::Hierarchy hierarchy, const float letterSpacing)
        : _hierarchy((std::move(hierarchy))), _letter_spacing(letterSpacing) {
    }

    bool update(uint64_t timestamp) override {
        doFlowLayout(_hierarchy._child_nodes, _letter_spacing, 0, 0);
        return false;
    }

private:
    Layout::Hierarchy _hierarchy;
    float _letter_spacing;
};

class UpdatableCenter final : public Updatable {
public:
    UpdatableCenter(Layout::Hierarchy hierarchy, Size size, const float letterSpacing)
        : _hierarchy((std::move(hierarchy))), _letter_spacing(letterSpacing), _size(std::move(size)) {
    }

    bool update(uint64_t timestamp) override {
        const float flowX = (_size.widthAsFloat() - _hierarchy._node->size()->x()) / 2;
        const float flowY = (_size.heightAsFloat() - _hierarchy._node->size()->y()) / 2;
        doFlowLayout(_hierarchy._child_nodes, _letter_spacing, flowX, flowY);
        return false;
    }

private:
    Layout::Hierarchy _hierarchy;
    float _letter_spacing;
    Size _size;
};

class UpdatableFlexEnd final : public Updatable {
public:
    UpdatableFlexEnd(Layout::Hierarchy hierarchy, Size size, const float letterSpacing)
        : _hierarchy((std::move(hierarchy))), _letter_spacing(letterSpacing), _size(std::move(size)) {
    }

    bool update(uint64_t timestamp) override {
        const float flowX = _size.widthAsFloat() - _hierarchy._node->size()->x();
        const float flowY = (_size.heightAsFloat() - _hierarchy._node->size()->y()) / 2;
        doFlowLayout(_hierarchy._child_nodes, _letter_spacing, flowX, flowY);
        return false;
    }

private:
    Layout::Hierarchy _hierarchy;
    float _letter_spacing;
    Size _size;
};

class BoundariesWidth final : public Numeric {
public:
    BoundariesWidth(sp<Boundaries> boundaries)
        : _boundaries(std::move(boundaries)) {
    }

    bool update(const uint64_t timestamp) override
    {
        return _boundaries->update(timestamp);
    }

    float val() override
    {
        return _boundaries->size()->val().x();
    }

private:
    sp<Boundaries> _boundaries;
};

class UpdatableParagraph final : public Updatable {
public:
    UpdatableParagraph(Layout::Hierarchy hierarchy, const float letterSpacing, LayoutLength lineHeight, sp<Numeric> layoutWidth, sp<Size> textSize)
        : _hierarchy((std::move(hierarchy))), _letter_spacing(letterSpacing), _line_height(lineHeight.isAuto() ? LayoutLength(100, LayoutLength::LENGTH_TYPE_PERCENTAGE) : std::move(lineHeight)), _line_indent(0), _layout_width(std::move(layoutWidth)), _text_size(std::move(textSize))
    {
        doParagraphLayout();
    }

    bool update(const uint64_t timestamp) override
    {
        if(!_layout_width->update(timestamp))
            return false;

        doParagraphLayout();
        return true;
    }

    void doParagraphLayout() const
    {
        float flowx = 0;
        float flowy = 0;
        const float width = _layout_width->val();
        const float boundary = flowx + width;
        const Vector<Layout::Hierarchy>& childNodes = _hierarchy._child_nodes;

        size_t begin = 0, end = 1;
        const float paragraphX = flowx;
        float charMaxHeight = std::numeric_limits<float>::min();
        for(const Layout::Hierarchy& i : childNodes)
        {
            const Character& currentChar = *static_cast<Character*>(i._node->_tag);
            charMaxHeight = std::max(currentChar._glyph->occupySize().y(), charMaxHeight);
            if(const bool allowLineBreak = currentChar._is_cjk || currentChar._is_word_break; end == childNodes.size() || allowLineBreak)
            {
                const float beginWidth = begin > 0 ? static_cast<Character*>(childNodes.at(begin - 1)._node->_tag)->_width_integral : 0;
                if(const float placingWidth = currentChar._width_integral - beginWidth; flowx + placingWidth > boundary && allowLineBreak)
                {
                    if(flowx != _line_indent)
                    {
                        const float layoutDirection = Ark::instance().applicationContext()->renderEngine()->toLayoutDirection(_line_height.toPixelLength(charMaxHeight));
                        flowy += layoutDirection;
                        flowx = paragraphX + _line_indent;
                    }
                    else
                        LOGW("No other choices, placing word out of boundary(%.2f)", boundary);
                }

                place(childNodes, begin, end, flowx, flowy);
                begin = end;
            }
            ++ end;
        }

        _text_size->setWidth(width);
        _text_size->setHeight(flowy + charMaxHeight);
    }

    void place(const Vector<Layout::Hierarchy>& childNodes, const size_t begin, const size_t end, float& flowx, float& flowy) const
    {
        for(size_t i = begin; i < end; ++i)
        {
            Layout::Node& node = childNodes.at(i)._node;
            node.setOffsetPosition({flowx, flowy});
            flowx += _letter_spacing + node.size()->x();
        }
    }

private:
    Layout::Hierarchy _hierarchy;
    float _letter_spacing;
    LayoutLength _line_height;
    float _line_indent;
    sp<Numeric> _layout_width;
    sp<Size> _text_size;
};

class LayoutLabel final : public Layout {
public:
    LayoutLabel(const float letterSpacing)
        : _letter_spacing(letterSpacing) {
    }

    sp<Updatable> inflate(Hierarchy hierarchy) override
    {
        if(hierarchy._node->_layout_param)
        {
            const LayoutParam& lp = hierarchy._node->_layout_param;
            Size size(lp.width().value().val(), lp.height().value().val());
            switch(lp.justifyContent()) {
                case LayoutParam::JUSTIFY_CONTENT_CENTER:
                    return sp<Updatable>::make<UpdatableCenter>(std::move(hierarchy), std::move(size), _letter_spacing);
                case LayoutParam::JUSTIFY_CONTENT_FLEX_END:
                    return sp<Updatable>::make<UpdatableFlexEnd>(std::move(hierarchy), std::move(size), _letter_spacing);
                case LayoutParam::JUSTIFY_CONTENT_FLEX_START:
                default:
                    break;
            }
        }
        return sp<Updatable>::make<UpdatableFlexStart>(std::move(hierarchy), _letter_spacing);
    }

private:
    float _letter_spacing;
};

class LayoutParagraph final : public Layout {
public:
    LayoutParagraph(const float letterSpacing, LayoutLength lineHeight, sp<Numeric> layoutWidth, sp<Size> textSize)
        : _letter_spacing(letterSpacing), _line_height(std::move(lineHeight)), _layout_width(std::move(layoutWidth)), _text_size(std::move(textSize))
    {
    }

    sp<Updatable> inflate(Hierarchy hierarchy) override
    {
        return sp<Updatable>::make<UpdatableParagraph>(std::move(hierarchy), _letter_spacing, _line_height, _layout_width, _text_size);
    }

private:
    float _letter_spacing;
    LayoutLength _line_height;
    sp<Numeric> _layout_width;
    sp<Size> _text_size;
};

}

struct Text::Content {
    Content(sp<RenderLayer> renderLayer, sp<StringVar> text, sp<Vec3> position, sp<LayoutParam> layoutParam, sp<GlyphMaker> glyphMaker, sp<Mat4> transform, const float letterSpacing, LayoutLength lineHeight, const float lineIndent)
        : _render_layer(std::move(renderLayer)), _text(text ? std::move(text) : StringType::create()), _position(std::move(position)), _layout_param(std::move(layoutParam)),
          _glyph_maker(std::move(glyphMaker)), _transform(std::move(transform)), _letter_spacing(letterSpacing), _layout_direction(Ark::instance().applicationContext()->renderEngine()->toLayoutDirection(1.0f)),
          _line_height(std::move(lineHeight)), _line_indent(lineIndent), _size(sp<Size>::make(0.0f, 0.0f))
    {
        setText(Strings::fromUTF8(_text->val()));
    }

    bool update(const uint64_t timestamp)
    {
        const bool contentDirty = _text->update(timestamp);
        const bool layoutDirty = _timestamp.update(timestamp);
        if(contentDirty)
            setText(Strings::fromUTF8(_text->val()));
        else if(layoutDirty)
            updateLayoutContent();
        return contentDirty || layoutDirty || UpdatableUtil::update(timestamp, _updatable_layout);
    }

    void setText(std::wstring text)
    {
        _text_unicode = std::move(text);
        createContent();
    }

    void createContent()
    {
        _glyphs = makeGlyphs(_glyph_maker, _text_unicode);
        _layout_chars = toLayoutCharacters(_glyphs, _render_layer->modelLoader());
        createLayerContent();
    }

    V2 doAutoWidthLayout() const
    {
        float lineHeight = 0;
        float flowx = _layout_chars.empty() ? 0 : -_letter_spacing;
        for(const Character& i : _layout_chars)
        {
            const Model& model = i._model;
            const V2 bitmapOccupySize(model.occupy()->size()->val());
            const V2 bitmapContentSize(model.content()->size()->val());
            lineHeight = std::max(bitmapOccupySize.y(), lineHeight);
            i._glyph->setSize(bitmapOccupySize, bitmapContentSize);
            flowx += bitmapOccupySize.x() + _letter_spacing;
        }
        return {flowx, lineHeight};
    }

    void updateLayoutSize() const
    {
        const V2 size = doAutoWidthLayout();
        _size->setWidth(size.x());
        _size->setHeight(size.y());
    }

    void createLayerContent()
    {
        updateLayoutSize();

        _render_objects.clear();
        for(const sp<Glyph>& i : _glyphs)
            _render_objects.push_back(i->toRenderObject());

        updateLayoutContent();
    }

    void updateLayoutContent()
    {
        if(_layer_context)
            _layer_context->clear();
        else
            _layer_context = _render_layer->makeLayerContext(nullptr, _position.toVar(), nullptr, nullptr);

        Layout::Hierarchy hierarchy = makeHierarchy();
        DASSERT(_render_objects.size() == hierarchy._child_nodes.size());
        for(size_t i = 0; i < _render_objects.size(); ++i)
        {
            sp<Renderable> renderable = sp<Renderable>::make<RenderableCharacter>(_render_objects.at(i), hierarchy._child_nodes.at(i)._node, _layout_chars.at(i)._offset);
            if(_transform)
                renderable = sp<Renderable>::make<RenderableWithTransform>(std::move(renderable), _transform);
            _layer_context->pushBack(std::move(renderable));
        }

        const sp<Layout> layout = makeTextLayout();
        _updatable_layout = layout->inflate(std::move(hierarchy));
    }

    sp<Layout> makeTextLayout() const
    {
        if(_layout_param && _layout_param->isWrapContent())
            return sp<Layout>::make<LayoutParagraph>(_letter_spacing, _line_height, _boundaries ? sp<Numeric>::make<BoundariesWidth>(_boundaries) : _layout_param->width().value().toVar(), _size);

        return sp<Layout>::make<LayoutLabel>(_letter_spacing);
    }

    Layout::Hierarchy makeHierarchy()
    {
        Layout::Hierarchy hierarchy = {sp<Layout::Node>::make(_layout_param)};
        hierarchy._node->setSize(_size->val());
        for(Character& i : _layout_chars)
        {
            sp<Layout::Node> node = sp<Layout::Node>::make(sp<LayoutParam>::make(i._glyph->occupySize().x(), i._glyph->occupySize().y()), &i);
            node->_tag = &i;
            hierarchy._child_nodes.push_back({std::move(node)});
        }
        return hierarchy;
    }

    void rewireBoundaries(sp<Boundaries> boundaries, const bool rewirePosition)
    {
        if(!_position || rewirePosition)
            _position.reset(boundaries->aabbMin());

        _boundaries = std::move(boundaries);
        _timestamp.markDirty();
    }

    sp<RenderLayer> _render_layer;
    sp<StringVar> _text;
    SafeVar<Vec3> _position;
    sp<LayoutParam> _layout_param;
    sp<GlyphMaker> _glyph_maker;
    sp<Mat4> _transform;

    float _letter_spacing;
    float _layout_direction;
    LayoutLength _line_height;
    float _line_indent;

    sp<Size> _size;
    sp<Boundaries> _boundaries;

    std::wstring _text_unicode;
    Vector<sp<Glyph>> _glyphs;
    Vector<Character> _layout_chars;
    Vector<sp<RenderObject>> _render_objects;

    sp<LayerContext> _layer_context;
    sp<Updatable> _updatable_layout;

    Timestamp _timestamp;
};

class RenderBatchContent final : public RenderBatch {
public:
    RenderBatchContent(sp<Text::Content> content, sp<Boolean> discarded)
        : RenderBatch(std::move(discarded)), _content(std::move(content)) {
    }

    Vector<sp<LayerContext>>& snapshot(const RenderRequest& renderRequest) override {
        _layer_contexts.clear();
        _content->update(renderRequest.timestamp());
        _layer_contexts.push_back(_content->_layer_context);
        return _layer_contexts;
    }

private:
    sp<Text::Content> _content;
    Vector<sp<LayerContext>> _layer_contexts;
};

Text::Text(sp<RenderLayer> renderLayer, sp<StringVar> text, sp<Vec3> position, sp<LayoutParam> layoutParam, sp<GlyphMaker> glyphMaker, sp<Mat4> transform, float letterSpacing, LayoutLength lineHeight, float lineIndent)
    : _content(sp<Content>::make(std::move(renderLayer), std::move(text), std::move(position), std::move(layoutParam), glyphMaker ? std::move(glyphMaker) : sp<GlyphMaker>::make<GlyphMakerSpan>(std::move(nullptr)), std::move(transform), letterSpacing, std::move(lineHeight), lineIndent))
{
}

void Text::onWire(const WiringContext& context, const Box& self)
{
    if(const sp<View> view = context.getComponent<View>())
    {
        sp<LayoutParam> layoutParam = view->layoutParam();
        sp<Boundaries> boundaries = view->makeBoundaries();
        _content->rewireBoundaries(std::move(boundaries), false);
        if(!layoutParam->isWrapContent())
        {
            if(layoutParam->width().type() == LayoutLength::LENGTH_TYPE_AUTO)
                layoutParam->setWidth({size()->width(), LayoutLength::LENGTH_TYPE_PIXEL});
            if(layoutParam->height().type() == LayoutLength::LENGTH_TYPE_AUTO)
                layoutParam->setHeight({size()->height(), LayoutLength::LENGTH_TYPE_PIXEL});
        }
        _content->_layout_param = std::move(layoutParam);
    }
    else if(sp<Vec3> translation = context.getComponent<Translation>(); translation && !_content->_position)
        setPosition(std::move(translation));

    if(sp<Mat4> transform = context.getComponent<Transform>(); transform && !_content->_transform)
        setTransform(std::move(transform));

    if(const sp<Node> node = context.getComponent<Node>())
    {
        sp<Mat4> matrix = transform();
        matrix = matrix ? Mat4Type::matmul(std::move(matrix), node->localMatrix()) : sp<Mat4>::make<Mat4::Const>(node->localMatrix());
        if(matrix)
            setTransform(std::move(matrix));
    }

    show(context.getComponent<Discarded>());
}

const Vector<sp<RenderObject>>& Text::contents() const
{
    return _content->_render_objects;
}

sp<Vec3> Text::position() const
{
    return _content->_position.toVar();
}

void Text::setPosition(sp<Vec3> position)
{
    _content->_position.reset(std::move(position));
}

const sp<Mat4>& Text::transform() const
{
    return _content->_transform;
}

void Text::setTransform(sp<Mat4> transform)
{
    _content->_transform = std::move(transform);
    _content->_timestamp.markDirty();
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
    _content->rewireBoundaries(std::move(boundaries), true);
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

    _render_batch = sp<RenderBatch>::make<RenderBatchContent>(_content, discarded ? std::move(discarded) : sp<Boolean>::make<BooleanByWeakRef<Content>>(_content, 0));
    _content->_render_layer->addRenderBatch(_render_batch);
}

void Text::hide()
{
    if(_render_batch)
        _render_batch->setDiscarded(Global<Constants>()->BOOLEAN_TRUE);
    _render_batch = nullptr;
}

Text::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _render_layer(factory.ensureBuilder<RenderLayer>(manifest, constants::RENDER_LAYER)), _text(factory.getBuilder<StringVar>(manifest, constants::TEXT)), _font(factory.getBuilder<Font>(manifest, constants::FONT)), _position(factory.getBuilder<Vec3>(manifest, constants::POSITION)),
      _layout_param(factory.getBuilder<LayoutParam>(manifest, constants::LAYOUT_PARAM)), _glyph_maker(factory.getBuilder<GlyphMaker>(manifest, "glyph-maker")), _transform(factory.getBuilder<Mat4>(manifest, constants::TRANSFORM)), _letter_spacing(factory.getBuilder<Numeric>(manifest, "letter-spacing")),
      _line_height(factory.getIBuilder<LayoutLength>(manifest, "line-height"), LayoutLength(100.0f, LayoutLength::LENGTH_TYPE_PERCENTAGE)), _line_indent(Documents::getAttribute<float>(manifest, "line-indent", 0.0f))
{
}

sp<Text> Text::BUILDER::build(const Scope& args)
{
    sp<GlyphMaker> glyphMaker = _glyph_maker.build(args);
    float letterSpacing = _letter_spacing ? _letter_spacing.build(args)->val() : 0.0f;
    return sp<Text>::make(_render_layer->build(args), _text.build(args), _position.build(args), _layout_param.build(args), glyphMaker ? std::move(glyphMaker) : sp<GlyphMaker>::make<GlyphMakerSpan>(_font.build(args)), _transform.build(args), letterSpacing, _line_height.build(args), _line_indent);
}

Text::BUILDER_WIRABLE::BUILDER_WIRABLE(BeanFactory& factory, const document& manifest)
    : _text(factory.ensureBuilder<Text>(manifest))
{
}

sp<Wirable> Text::BUILDER_WIRABLE::build(const Scope& args)
{
    return _text->build(args);
}

}
