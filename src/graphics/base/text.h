#pragma once

#include <deque>
#include <string>
#include <vector>

#include "core/base/api.h"
#include "core/base/bean_factory.h"
#include "core/inf/builder.h"
#include "core/types/safe_ptr.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/inf/renderable.h"
#include "graphics/inf/render_batch.h"


namespace ark {

class ARK_API Text {
public:
//  [[script::bindings::auto]]
    Text(sp<RenderLayer> renderLayer, sp<StringVar> content = nullptr, sp<GlyphMaker> glyphMaker = nullptr, float textScale = 1.0f, float letterSpacing = 0.0f, float lineHeight = 0.0f, float lineIndent = 0.0f);

//  [[script::bindings::property]]
    const std::vector<sp<RenderObject>>& contents() const;

//  [[script::bindings::property]]
    sp<Vec3> position() const;
//  [[script::bindings::property]]
    void setPosition(sp<Vec3> position);

//  [[script::bindings::property]]
    const sp<Size>& size() const;

//  [[script::bindings::property]]
    const sp<Size>& layoutSize() const;
//  [[script::bindings::property]]
    void setLayoutSize(sp<Size> layoutSize);

//  [[script::bindings::property]]
    const std::wstring& text() const;
//  [[script::bindings::property]]
    void setText(std::wstring text);

//  [[script::bindings::auto]]
    void show(sp<Boolean> disposed = nullptr);

//  [[script::bindings::auto]]
    void setRichText(std::wstring richText, const sp<ResourceLoader>& resourceLoader = nullptr, const Scope& args = Scope());

    bool update(uint64_t timestamp) const;

//[[plugin::builder]]
    class BUILDER : public Builder<Text> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        sp<Text> build(const Scope& args) override;

    private:
        sp<Builder<RenderLayer>> _render_layer;
        SafePtr<Builder<StringVar>> _text;
        SafePtr<Builder<GlyphMaker>> _glyph_maker;

        sp<Builder<String>> _text_scale;
        sp<Builder<Numeric>> _letter_spacing;
        float _line_height;
        float _line_indent;
    };

private:
    typedef std::vector<sp<Glyph>> GlyphContents;

    struct LayoutChar {
        LayoutChar(sp<Model> model, float widthIntegral, bool isCJK, bool isWordBreak, bool isLineBreak);

        sp<Model> _model;
        float _width_integral;
        bool _is_cjk;
        bool _is_word_break;
        bool _is_line_break;
    };

    static bool isCJK(int32_t c);
    static bool isWordBreaker(wchar_t c);
    static GlyphContents makeGlyphs(GlyphMaker& gm, const std::wstring& text);

    class Content {
    public:
        Content(sp<RenderLayer> renderLayer, sp<StringVar> string, sp<GlyphMaker> glyphMaker, float textScale, float letterSpacing, float lineHeight, float lineIndent);

        bool update(uint64_t timestamp);

        void setText(std::wstring text);
        void setRichText(std::wstring richText, const sp<ResourceLoader>& resourceLoader, const Scope& args);

        void setRenderObjects(std::vector<sp<RenderObject>> renderObjects);
        void reload();

        void layoutContent();

        void createContent();
        void createRichContent(const Scope& args, BeanFactory& factory);

    private:
        float doLayoutContent(GlyphContents& cm, float& flowx, float& flowy, float boundary);
        float doCreateRichContent(GlyphContents& cm, GlyphMaker& gm, const document& richtext, BeanFactory& factory, const Scope& args, float& flowx, float& flowy, float boundary);
        float doLayoutWithBoundary(GlyphContents& cm, float& flowx, float& flowy, float boundary);
        float doLayoutWithoutBoundary(GlyphContents& cm, float& flowx, float flowy);

        void createLayerContent(float width, float height);
        void place(GlyphContents& cm, const std::vector<Text::LayoutChar>& layouts, size_t begin, size_t end, float& flowx, float flowy);
        void placeOne(Glyph& glyph, const Model& model, float& flowx, float flowy, float* fontHeight = nullptr);

        void nextLine(float fontHeight, float& flowx, float& flowy) const;

        float getFlowY() const;
        float getLayoutBoundary() const;

        std::vector<Text::LayoutChar> toLayoutCharacters(const GlyphContents& glyphs) const;

    private:
        sp<RenderLayer> _render_layer;
        sp<StringVar> _string;
        sp<LayerContext> _layer_context;
        sp<GlyphMaker> _glyph_maker;

        float _text_scale;
        float _letter_spacing;
        float _layout_direction;
        float _line_height;
        float _line_indent;

        sp<Size> _size;
        sp<Size> _layout_size;

        std::wstring _text_unicode;
        std::vector<sp<Glyph>> _glyphs;

        sp<VariableWrapper<V3>> _position;
        std::vector<sp<RenderObject>> _render_objects;

        friend class Text;
    };

    class RenderBatchContent : public RenderBatch {
    public:
        RenderBatchContent(sp<Content> content, sp<Boolean> disposed);

        virtual std::vector<sp<LayerContext>>& snapshot(const RenderRequest& renderRequest) override;

    private:
        sp<Content> _content;
        std::vector<sp<LayerContext>> _layer_contexts;
    };

private:
    sp<RenderLayer> _render_layer;

    sp<Content> _content;
    sp<RenderBatch> _render_batch;
};

}
