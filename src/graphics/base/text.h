#ifndef ARK_GRAPHICS_BASE_CHARACTERS_H_
#define ARK_GRAPHICS_BASE_CHARACTERS_H_

#include <string>
#include <vector>

#include "core/base/api.h"
#include "core/base/bean_factory.h"
#include "core/base/bean_factory_weak_ref.h"
#include "core/inf/builder.h"
#include "core/types/safe_ptr.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/base/metrics.h"
#include "graphics/inf/renderer.h"
#include "graphics/inf/renderable.h"
#include "graphics/inf/renderable_batch.h"


namespace ark {

//[[script::bindings::extends(Renderer)]]
class ARK_API Text : public Renderer {
public:
[[deprecated]]
//  [[script::bindings::auto]]
    Text(const sp<Layer>& layer, sp<StringVar> text = nullptr, float textScale = 1.0f, float letterSpacing = 0.0f, float lineHeight = 0.0f, float lineIndent = 0.0f);
[[deprecated]]
//  [[script::bindings::auto]]
    Text(const sp<LayerContext>& layer, sp<StringVar> text = nullptr, float textScale = 1.0f, float letterSpacing = 0.0f, float lineHeight = 0.0f, float lineIndent = 0.0f);
//  [[script::bindings::auto]]
    Text(sp<RenderLayer> renderLayer, sp<StringVar> text = nullptr, float textScale = 1.0f, float letterSpacing = 0.0f, float lineHeight = 0.0f, float lineIndent = 0.0f);
    Text(sp<RenderLayer> renderLayer, sp<LayerContext> layerContext, sp<StringVar> text, sp<GlyphMaker> glyphMaker, float textScale, float letterSpacing, float lineHeight, float lineIndent);

//  [[script::bindings::property]]
    const std::vector<sp<RenderObject>>& contents() const;

//  [[script::bindings::property]]
    const sp<Vec3>& position() const;
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

    [[deprecated]]
    virtual void render(RenderRequest& renderRequest, const V3& position) override;

    bool update(uint64_t timestamp) const;

//[[plugin::builder]]
    class BUILDER : public Builder<Text> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        virtual sp<Text> build(const Scope& args) override;

    private:
        BeanFactory _bean_factory;
        SafePtr<Builder<StringVar>> _text;
        SafePtr<Builder<RenderLayer>> _render_layer;
        sp<Builder<LayerContext>> _layer_context;
        SafePtr<Builder<GlyphMaker>> _glyph_maker;

        sp<Builder<String>> _text_scale;
        sp<Builder<Numeric>> _letter_spacing;
        float _line_height;
        float _line_indent;
    };

private:
    typedef std::vector<sp<Glyph>> GlyphContents;

    struct LayoutChar {
        LayoutChar(const Metrics& metrics, float widthIntegral, bool isCJK, bool isWordBreak, bool isLineBreak);

        Metrics _metrics;
        float _width_integral;
        bool _is_cjk;
        bool _is_word_break;
        bool _is_line_break;
    };

    static bool isCJK(int32_t c);
    static bool isWordBreaker(wchar_t c);
    static GlyphContents makeGlyphs(GlyphMaker& gm, const std::wstring& text);

    class Content : public RenderableBatch {
    public:
        Content(const sp<LayerContext>& layerContext, sp<StringVar> string, sp<GlyphMaker> glyphMaker, sp<ModelLoader> modelLoader, float textScale, float letterSpacing, float lineHeight, float lineIndent);

        virtual bool preSnapshot(const RenderRequest& renderRequest, LayerContext& lc) override;
        virtual void snapshot(const RenderRequest& renderRequest, const LayerContext& lc, RenderLayerSnapshot& output) override;

        SafeVar<Vec3>& position();

        bool update(const sp<LayerContext>& layerContext, uint64_t timestamp);

        void setText(const sp<LayerContext>& layerContext, std::wstring text);
        void setRichText(const sp<LayerContext>& layerContext, std::wstring richText, const sp<ResourceLoader>& resourceLoader, const Scope& args);

        const std::vector<sp<RenderObject>>& renderObjects() const;
        void setRenderObjects(std::vector<sp<RenderObject>> renderObjects);

        void layoutContent();

        void createContent(const sp<LayerContext>& layerContext);
        void createRichContent(const sp<LayerContext>& layerContext, const Scope& args, BeanFactory& factory);

    private:
        float doLayoutContent(GlyphContents& cm, float& flowx, float& flowy, float boundary);
        float doCreateRichContent(GlyphContents& cm, GlyphMaker& gm, const document& richtext, BeanFactory& factory, const Scope& args, float& flowx, float& flowy, float boundary);
        float doLayoutWithBoundary(GlyphContents& cm, float& flowx, float& flowy, float boundary);
        float doLayoutWithoutBoundary(GlyphContents& cm, float& flowx, float flowy);

        void createLayerContent(const sp<LayerContext>& layerContext, float width, float height);
        void place(GlyphContents& cm, const std::vector<Text::LayoutChar>& layouts, size_t begin, size_t end, float& flowx, float flowy);
        void placeOne(Glyph& glyph, const Metrics& metrics, float& flowx, float flowy, float* fontHeight = nullptr);

        void nextLine(float fontHeight, float& flowx, float& flowy) const;

        float getFlowY() const;
        float getLayoutBoundary() const;

        std::vector<Text::LayoutChar> getCharacterMetrics(const GlyphContents& glyphs) const;

    private:
        sp<StringVar> _string;
        sp<GlyphMaker> _glyph_maker;
        sp<ModelLoader> _model_loader;

        float _text_scale;
        float _letter_spacing;
        float _layout_direction;
        float _line_height;
        float _line_indent;

        sp<Size> _size;
        sp<Size> _layout_size;

        std::wstring _text_unicode;
        std::vector<sp<Glyph>> _glyphs;

        SafeVar<Vec3> _position;
        std::vector<sp<RenderObject>> _render_objects;
        std::vector<Renderable::StateBits> _render_object_states;

        std::vector<sp<RenderablePassive>> _renderables;

        bool _needs_reload;

        friend class Text;
    };

private:
    sp<RenderLayer> _render_layer;
    sp<LayerContext> _layer_context;

    sp<Content> _content;
};

}

#endif
