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
#include "graphics/impl/glyph_maker/glyph_maker_font.h"
#include "graphics/inf/glyph_maker.h"
#include "graphics/inf/layout.h"
#include "graphics/util/vec2_type.h"

#include "renderer/base/model.h"
#include "renderer/base/render_engine.h"
#include "renderer/base/resource_loader_context.h"
#include "renderer/inf/model_loader.h"

#include "app/base/application_context.h"
#include "app/view/view.h"

namespace ark {

namespace {

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
    return std::iswblank(c);
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

Vector<Character> toLayoutCharacters(const Vector<sp<Glyph>>& glyphs, ModelLoader& modelLoader)
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
            const V2 occupy(model->occupy()->size()->val());
            integral += occupy.x();
            layoutChars.push_back({i, model, offset, integral, iscjk, iswordbreak, isLineBreak});
            i->setSize(occupy, model->content()->size()->val());
        }
        else
        {
            const int32_t type = c;
            sp<Model> model = modelLoader.loadModel(i->font() ? i->font()->combine(type) : type);
            const V2 offset = getCharacterOffset(model);
            const Boundaries& m = model->occupy();
            const bool iscjk = isCJK(c);
            const bool iswordbreak = isWordBreaker(c);
            integral += m.size()->val().x();
            mmap.insert(std::make_pair(c, std::make_tuple(model, offset, iscjk, iswordbreak)));
            i->setSize(model->occupy()->size()->val(), model->content()->size()->val());
            layoutChars.push_back({i, std::move(model), offset, integral, iscjk, iswordbreak, isLineBreak});
        }
    }
    return layoutChars;
}

class LayoutNodeSize final : public Vec2 {
public:
    LayoutNodeSize(sp<Layout::Node> layoutNode)
        : _layout_node(std::move(layoutNode)) {
    }

    bool update(uint32_t tick) override
    {
        return _layout_node->size().update(tick);
    }

    V2 val() override
    {
        return _layout_node->size().val();
    }

private:
    sp<Layout::Node> _layout_node;
};

struct LayoutInfo {
    LayoutInfo(sp<LayoutParam> layoutParam, sp<Vec2> scale, const float letterSpacing, const float lineIndent, LayoutLength lineHeight)
        : _scale(std::move(scale), {1.0f, 1.0f}), _letter_spacing(letterSpacing), _line_indent(lineIndent), _line_height(lineHeight.isAuto() ? LayoutLength(100, LayoutLength::LENGTH_TYPE_PERCENTAGE) : std::move(lineHeight)),
          _auto_width(sp<Numeric::Impl>::make(0)), _auto_height(sp<Numeric::Impl>::make(0))
    {
        setLayoutNode(sp<Layout::Node>::make(std::move(layoutParam)));
    }

    void setLayoutNode(sp<Layout::Node> layoutNode)
    {
        _layout_size = Vec2Type::toSize(sp<Vec2>::make<LayoutNodeSize>(layoutNode));
        _layout_node = std::move(layoutNode);
        _size = sp<Size>::make(_layout_node->isAutoWidth() ? _auto_width.cast<Numeric>() : _layout_size->width(), _layout_node->isAutoHeight() ? _auto_height.cast<Numeric>() : _layout_size->height());

        if(_layout_node->isAutoWidth())
            _layout_node->setAutoWidth(_auto_width);
        if(_layout_node->isAutoHeight())
            _layout_node->setAutoHeight(_auto_height);
    }

    bool isWidthWrapContent() const
    {
        const sp<LayoutParam>& layoutParam = _layout_node->_layout_param;
        return layoutParam && layoutParam->isWrapContent() && !layoutParam->width().isAuto();
    }

    const sp<LayoutParam>& layoutParam() const
    {
        return _layout_node->_layout_param;
    }

    OptionalVar<Vec2> _scale;

    float _letter_spacing;
    float _line_indent;
    LayoutLength _line_height;

    sp<Layout::Node> _layout_node;
    sp<Size> _layout_size;
    sp<Size> _size;

    sp<Numeric::Impl> _auto_width;
    sp<Numeric::Impl> _auto_height;

    sp<Boundaries> _boundaries;
};

class RenderableCharacter final : public Renderable {
public:
    RenderableCharacter(sp<Renderable> delegate, sp<Layout::Node> layoutNode, sp<LayoutInfo> layoutInfo, const V2 offsetPosition)
        : _delegate(std::move(delegate)), _layout_node(std::move(layoutNode)), _layout_info(std::move(layoutInfo)), _offset_position(offsetPosition) {
    }

    State updateState(const RenderRequest& renderRequest) override {
        const State state = _delegate->updateState(renderRequest);
        if(_layout_node->update(renderRequest.tick()))
            return state | RENDERABLE_STATE_DIRTY;
        return state;
    }

    Snapshot snapshot(const LayerContextSnapshot& snapshotContext, const RenderRequest& renderRequest, const State state) override {
        Snapshot snapshot = _delegate->snapshot(snapshotContext, renderRequest, state);
        snapshot._position += V3(_layout_node->offsetPosition() + _offset_position, 0);
        snapshot._size *= V3(_layout_info->_scale.val(), 1.0f);
        return snapshot;
    }

private:
    sp<Renderable> _delegate;
    sp<Layout::Node> _layout_node;
    sp<LayoutInfo> _layout_info;
    V2 _offset_position;
};

V2 doFlexLayout(const Vector<Layout::Hierarchy>& childNodes, const LayoutInfo& layoutInfo)
{
    const V2 scale = layoutInfo._scale.val();
    const float letterSpacing = layoutInfo._letter_spacing;
    float lineHeight = 0;
    float flowX = 0;
    for(const Layout::Hierarchy& i : childNodes)
    {
        Layout::Node& node = i._node;
        node.setOffsetPosition(V2(flowX, 0));
        flowX += (letterSpacing + node.size()->x()) * scale.x();
        lineHeight = std::max(lineHeight, node.size()->y() * scale.y());
    }

    const V2 size(flowX, layoutInfo._line_height.toPixelLength(lineHeight));
    layoutInfo._auto_width->set(size.x());
    layoutInfo._auto_height->set(size.y());
    return size;
}

class UpdatableLabel : public Updatable {
public:
    UpdatableLabel(Layout::Hierarchy hierarchy, const sp<LayoutInfo>& layoutInfo)
        : _hierarchy((std::move(hierarchy))), _layout_info(layoutInfo) {
    }

protected:
    void addOffsetPosition(const V2 offset) const
    {
        for(const Layout::Hierarchy& i : _hierarchy._child_nodes)
        {
            Layout::Node& node = i._node;
            node.setOffsetPosition(node.offsetPosition().val() + offset);
        }
    }

protected:
    Layout::Hierarchy _hierarchy;
    sp<LayoutInfo> _layout_info;
};

class UpdatableFlexStart final : public UpdatableLabel {
public:
    UpdatableFlexStart(Layout::Hierarchy hierarchy, const sp<LayoutInfo>& layoutInfo)
        : UpdatableLabel(std::move(hierarchy), layoutInfo)
    {
    }

    bool update(uint32_t tick) override
    {
        if(_hierarchy._node->update(tick))
        {
            doFlexLayout(_hierarchy._child_nodes, _layout_info);
            return true;
        }
        return false;
    }
};

class UpdatableCenter final : public UpdatableLabel {
public:
    UpdatableCenter(Layout::Hierarchy hierarchy, const sp<LayoutInfo>& layoutInfo)
        : UpdatableLabel(std::move(hierarchy), layoutInfo) {
    }

    bool update(uint32_t tick) override
    {
        if(_hierarchy._node->update(tick))
        {
            const LayoutParam& layoutParam = _hierarchy._node->_layout_param;
            const V2 parentSize(layoutParam.width().value().val(), layoutParam.height().value().val());
            const V2 size = doFlexLayout(_hierarchy._child_nodes, _layout_info);
            addOffsetPosition((parentSize - size) * 0.5f);
            return true;
        }
        return false;
    }
};

class UpdatableFlexEnd final : public UpdatableLabel {
public:
    UpdatableFlexEnd(Layout::Hierarchy hierarchy, const sp<LayoutInfo>& layoutInfo)
        : UpdatableLabel(std::move(hierarchy), layoutInfo)
    {
    }

    bool update(uint32_t tick) override
    {
        if(_hierarchy._node->update(tick))
        {
            const LayoutParam& layoutParam = _hierarchy._node->_layout_param;
            const V2 parentSize(layoutParam.width().value().val(), layoutParam.height().value().val());
            const V2 size = doFlexLayout(_hierarchy._child_nodes, _layout_info);
            addOffsetPosition(V2(parentSize.x() - size.x(), (parentSize.y() - size.y()) * 0.5f));
            return true;
        }
        return false;
    }
};

class BoundariesWidth final : public Numeric {
public:
    BoundariesWidth(sp<Boundaries> boundaries)
        : _boundaries(std::move(boundaries)) {
    }

    bool update(uint32_t tick) override
    {
        return _boundaries->update(tick);
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
    UpdatableParagraph(Layout::Hierarchy hierarchy, sp<LayoutInfo> layoutInfo)
        : _hierarchy((std::move(hierarchy))), _layout_info(std::move(layoutInfo)), _layout_width(_layout_info->_boundaries ? sp<Numeric>::make<BoundariesWidth>(_layout_info->_boundaries) : _layout_info->layoutParam()->width().value().toVar())
    {
    }

    bool update(uint32_t tick) override
    {
        if(!_layout_width->update(tick))
            return false;

        doLayout();
        return true;
    }

    void doLayout() const
    {
        float flowx = 0;
        float flowy = 0;
        const V2 scale = _layout_info->_scale.val();
        const float boundary = flowx + _layout_width->val();
        const float lineIndent = _layout_info->_line_indent;
        const LayoutLength& lineHeight = _layout_info->_line_height;
        const Vector<Layout::Hierarchy>& childNodes = _hierarchy._child_nodes;

        size_t begin = 0, end = 1;
        const float paragraphX = flowx;
        float charMaxHeight = std::numeric_limits<float>::min();
        for(const Layout::Hierarchy& i : childNodes)
        {
            const Character& currentChar = *static_cast<Character*>(i._node->_tag);
            charMaxHeight = std::max(currentChar._glyph->occupySize().y() * scale.y(), charMaxHeight);
            if(const bool allowLineBreak = currentChar._is_cjk || currentChar._is_word_break || end == childNodes.size())
            {
                const float beginWidth = begin > 0 ? static_cast<Character*>(childNodes.at(begin - 1)._node->_tag)->_width_integral : 0;
                if(const float placingWidth = (currentChar._width_integral - beginWidth) * scale.x(); flowx + placingWidth > boundary && allowLineBreak)
                {
                    if(flowx != lineIndent)
                    {
                        const float layoutDirection = Ark::instance().applicationContext()->renderEngine()->toLayoutDirection(lineHeight.toPixelLength(charMaxHeight));
                        flowy += layoutDirection;
                        flowx = paragraphX + lineIndent;
                    }
                    else
                        LOGW("No other choices, placing word out of boundary(%.2f)", boundary);
                }

                place(childNodes, begin, end, flowx, flowy, scale);
                begin = end;
            }
            ++ end;
        }

        _layout_info->_auto_height->set(flowy + charMaxHeight);
    }

    void place(const Vector<Layout::Hierarchy>& childNodes, const size_t begin, const size_t end, float& flowx, float& flowy, const V2 scale) const
    {
        const float letterSpacing = _layout_info->_letter_spacing;

        for(size_t i = begin; i < end; ++i)
        {
            Layout::Node& node = childNodes.at(i)._node;
            node.setOffsetPosition({flowx, flowy});
            flowx += letterSpacing + node.size()->x() * scale.x();
        }
    }

private:
    Layout::Hierarchy _hierarchy;
    sp<LayoutInfo> _layout_info;
    sp<Numeric> _layout_width;
};

class LayoutText final : public Layout {
public:
    LayoutText(const sp<LayoutInfo>& layoutInfo)
        : _layout_info(layoutInfo) {
    }

    sp<Updatable> inflate(Hierarchy hierarchy) override
    {
        if(_layout_info->isWidthWrapContent())
            return sp<Updatable>::make<UpdatableParagraph>(std::move(hierarchy), _layout_info);

        if(hierarchy._node->_layout_param)
            switch(hierarchy._node->_layout_param->justifyContent()) {
                case LayoutParam::JUSTIFY_CONTENT_CENTER:
                    return sp<Updatable>::make<UpdatableCenter>(std::move(hierarchy), _layout_info);
                case LayoutParam::JUSTIFY_CONTENT_FLEX_END:
                    return sp<Updatable>::make<UpdatableFlexEnd>(std::move(hierarchy), _layout_info);
                case LayoutParam::JUSTIFY_CONTENT_FLEX_START:
                default:
                    break;
            }

        return sp<Updatable>::make<UpdatableFlexStart>(std::move(hierarchy), _layout_info);
    }

private:
    sp<LayoutInfo> _layout_info;
};

class LayoutTextInView final : public Layout {
public:
    LayoutTextInView(const sp<LayoutInfo>& layoutInfo)
        : _layout_info(layoutInfo) {
    }

    sp<Updatable> inflate(Hierarchy hierarchy) override
    {
        if(_layout_info->isWidthWrapContent())
            return sp<Updatable>::make<UpdatableParagraph>(std::move(hierarchy), _layout_info);

        return sp<Updatable>::make<UpdatableFlexStart>(std::move(hierarchy), _layout_info);
    }

private:
    sp<LayoutInfo> _layout_info;
};

}

struct Text::Content {
    Content(sp<RenderLayer> renderLayer, sp<StringVar> text, sp<Vec3> position, sp<LayoutParam> layoutParam, sp<Vec2> scale, sp<GlyphMaker> glyphMaker, const float letterSpacing, LayoutLength lineHeight, const float lineIndent)
        : _render_layer(std::move(renderLayer)), _text(text ? std::move(text) : StringType::create()), _position(std::move(position)), _layout_info(sp<LayoutInfo>::make(std::move(layoutParam), std::move(scale), letterSpacing, lineIndent, std::move(lineHeight))),
          _glyph_maker(std::move(glyphMaker)), _layout(sp<Layout>::make<LayoutText>(_layout_info))
    {
    }

    bool update(const uint32_t tick)
    {
        const bool contentDirty = _text->update(tick);
        const bool layoutDirty = _timestamp.update(tick);
        if(contentDirty)
            createContent(Strings::fromUTF8(_text->val()));
        else if(layoutDirty)
            updateLayoutContent();
        return contentDirty || layoutDirty || UpdatableUtil::update(tick, _updatable_layout);
    }

    void setText(const std::wstring& text)
    {
        _text = StringType::create(Strings::toUTF8(text));
        createContent(text);
    }

    void createContent(const std::wstring& text)
    {
        _glyphs = _glyph_maker->makeGlyphs(text);
        _layout_chars = toLayoutCharacters(_glyphs, _layer_context->modelLoader());
        createLayerContent();
    }

    void createLayerContent()
    {
        _render_objects.clear();
        for(const sp<Glyph>& i : _glyphs)
            _render_objects.push_back(i->toRenderObject());

        updateLayoutContent();
    }

    void updateLayoutContent()
    {
        _layer_context->clear();

        Layout::Hierarchy hierarchy = makeHierarchy();
        DASSERT(_render_objects.size() == hierarchy._child_nodes.size());
        for(size_t i = 0; i < _render_objects.size(); ++i)
            _layer_context->pushBack(sp<Renderable>::make<RenderableCharacter>(_render_objects.at(i), hierarchy._child_nodes.at(i)._node, _layout_info, _layout_chars.at(i)._offset));

        _updatable_layout = _layout->inflate(std::move(hierarchy));
        _updatable_layout->update(Timestamp::now());
    }

    Layout::Hierarchy makeHierarchy()
    {
        Layout::Hierarchy hierarchy = {_layout_info->_layout_node};
        hierarchy._node->setSize(_layout_info->_layout_size->val());
        for(Character& i : _layout_chars)
        {
            sp<Layout::Node> node = sp<Layout::Node>::make(nullptr, &i);
            node->setSize(i._glyph->occupySize());
            node->_tag = &i;
            hierarchy._child_nodes.push_back({std::move(node)});
        }
        return hierarchy;
    }

    void wireView(View& view)
    {
        sp<Boundaries> boundaries = view.makeBoundaries();
        setBoundaries(std::move(boundaries));
        _layout_info->setLayoutNode(view.layoutNode());
        _layout = sp<Layout>::make<LayoutTextInView>(_layout_info);
    }

    void setBoundaries(sp<Boundaries> boundaries)
    {
        if(!_position)
            _position.reset(boundaries->aabbMin());

        _layout_info->_boundaries = std::move(boundaries);
        _timestamp.markDirty();
    }

    sp<RenderLayer> _render_layer;
    sp<StringVar> _text;
    OptionalVar<Vec3> _position;
    sp<LayoutInfo> _layout_info;
    sp<GlyphMaker> _glyph_maker;
    sp<Layout> _layout;

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
        _content->update(renderRequest.tick());
        _layer_contexts.push_back(_content->_layer_context);
        return _layer_contexts;
    }

private:
    sp<Text::Content> _content;
    Vector<sp<LayerContext>> _layer_contexts;
};

Text::Text(sp<RenderLayer> renderLayer, sp<StringVar> text, sp<Vec3> position, sp<LayoutParam> layoutParam, sp<Vec2> scale, sp<GlyphMaker> glyphMaker, float letterSpacing, LayoutLength lineHeight, float lineIndent)
    : _content(sp<Content>::make(std::move(renderLayer), std::move(text), std::move(position), std::move(layoutParam), std::move(scale), glyphMaker ? std::move(glyphMaker) : sp<GlyphMaker>::make<GlyphMakerFont>(nullptr), letterSpacing, std::move(lineHeight), lineIndent))
{
}

void Text::onWire(const WiringContext& context, const Box& self)
{
    if(const sp<View> view = context.getComponent<View>())
        _content->wireView(*view);
    else if(sp<Vec3> translation = context.getComponent<Translation>(); translation && !_content->_position)
        setPosition(std::move(translation));

    show(context.getComponent<Discarded>(), context.getInterface<RenderLayer>());
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

sp<Vec2> Text::scale() const
{
    return _content->_layout_info->_scale.toVar();
}

void Text::setScale(sp<Vec2> scale)
{
    _content->_layout_info->_scale.reset(std::move(scale));
    _content->_timestamp.markDirty();
}

const sp<Size>& Text::size() const
{
    return _content->_layout_info->_size;
}

const sp<Boundaries>& Text::boundaries() const
{
    return _content->_layout_info->_boundaries;
}

std::wstring Text::text() const
{
    return Strings::fromUTF8(_content->_text->val());
}

void Text::setText(std::wstring text) const
{
    _content->setText(std::move(text));
}

void Text::show(sp<Boolean> discarded, const sp<RenderLayer>& renderLayer)
{
    discard();

    const sp<RenderLayer>& rl = renderLayer ? renderLayer : _content->_render_layer;
    CHECK(rl, "Must specify text's RenderLayer");
    _content->_layer_context = rl->makeLayerContext(nullptr, _content->_position.toVar(), nullptr, discarded);

    _content->update(Timestamp::now());
    _render_batch = sp<RenderBatch>::make<RenderBatchContent>(_content, discarded ? std::move(discarded) : sp<Boolean>::make<BooleanByWeakRef<Content>>(_content, 0));
    rl->addRenderBatch(_render_batch);
}

void Text::discard()
{
    if(_render_batch)
        _render_batch->setDiscarded(Global<Constants>()->BOOLEAN_TRUE);
    _render_batch = nullptr;
}

Text::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _render_layer(factory.getBuilder<RenderLayer>(manifest, constants::RENDER_LAYER)), _text(factory.getBuilder<StringVar>(manifest, constants::TEXT)), _font(factory.getBuilder<Font>(manifest, constants::FONT)), _position(factory.getBuilder<Vec3>(manifest, constants::POSITION)),
      _layout_param(factory.getBuilder<LayoutParam>(manifest, constants::LAYOUT_PARAM)), _scale(factory.getBuilder<Vec2>(manifest, constants::SCALE)), _glyph_maker(factory.getBuilder<GlyphMaker>(manifest, "glyph-maker")), _letter_spacing(factory.getBuilder<Numeric>(manifest, "letter-spacing")),
      _line_height(factory.getIBuilder<LayoutLength>(manifest, "line-height"), LayoutLength(100.0f, LayoutLength::LENGTH_TYPE_PERCENTAGE)), _line_indent(Documents::getAttribute<float>(manifest, "line-indent", 0.0f))
{
}

sp<Text> Text::BUILDER::build(const Scope& args)
{
    sp<GlyphMaker> glyphMaker = _glyph_maker.build(args);
    float letterSpacing = _letter_spacing ? _letter_spacing.build(args)->val() : 0.0f;
    return sp<Text>::make(_render_layer.build(args), _text.build(args), _position.build(args), _layout_param.build(args), _scale.build(args), glyphMaker ? std::move(glyphMaker) : sp<GlyphMaker>::make<GlyphMakerFont>(_font.build(args)), letterSpacing, _line_height.build(args), _line_indent);
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
