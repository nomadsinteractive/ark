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
class ARK_API Text : public Renderer, public Updatable {
public:
[[deprecated]]
//  [[script::bindings::auto]]
    Text(const sp<Layer>& layer, sp<StringVar> text = nullptr, float textScale = 1.0f, float letterSpacing = 0.0f, float lineHeight = 0.0f, float lineIndent = 0.0f);
[[deprecated]]
//  [[script::bindings::auto]]
    Text(const sp<LayerContext>& layer, sp<StringVar> text = nullptr, float textScale = 1.0f, float letterSpacing = 0.0f, float lineHeight = 0.0f, float lineIndent = 0.0f);
//  [[script::bindings::auto]]
    Text(sp<RenderLayer> renderLayer, sp<StringVar> text = nullptr, float textScale = 1.0f, float letterSpacing = 0.0f, float lineHeight = 0.0f, float lineIndent = 0.0f);
    Text(const BeanFactory& factory, sp<RenderLayer> renderLayer, const sp<LayerContext>& layerContext, sp<StringVar> text, const sp<GlyphMaker>& glyphMaker, float textScale, float letterSpacing, float lineHeight, float lineIndent);

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
    void setText(const std::wstring& text);

//  [[script::bindings::auto]]
    void show(sp<Boolean> disposed = nullptr);

//  [[script::bindings::auto]]
    void setRichText(const std::wstring& richText, const Scope& args);

    [[deprecated]]
    virtual void render(RenderRequest& renderRequest, const V3& position) override;

    virtual bool update(uint64_t timestamp) override;

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

    void createContent();
    void createRichContent(const Scope& args);
    float doLayoutWithBoundary(GlyphContents& cm, float& flowx, float& flowy, float boundary);
    float doLayoutWithoutBoundary(GlyphContents& cm, float& flowx, float flowy);

    float doLayoutContent(GlyphContents& cm, float& flowx, float& flowy, float boundary);
    float doCreateRichContent(GlyphContents& cm, GlyphMaker& gm, const document& richtext, BeanFactory& factory, const Scope& args, float& flowx, float& flowy, float boundary);
    void layoutContent();

    void createLayerContent(float width, float height);

    struct LayoutChar {
        LayoutChar(const Metrics& metrics, float widthIntegral, bool isCJK, bool isWordBreak, bool isLineBreak);

        Metrics _metrics;
        float _width_integral;
        bool _is_cjk;
        bool _is_word_break;
        bool _is_line_break;
    };

    void place(GlyphContents& cm, const std::vector<LayoutChar>& layouts, size_t begin, size_t end, float& flowx, float flowy);
    void placeOne(Glyph& cm, const Metrics& metrics, float& flowx, float flowy, float* fontHeight = nullptr);

    void nextLine(float fontHeight, float& flowx, float& flowy) const;
    float getFlowY() const;

    std::vector<LayoutChar> getCharacterMetrics(const GlyphContents& glyphs) const;
    bool isCJK(int32_t c) const;
    bool isWordBreaker(wchar_t c) const;

    GlyphContents makeGlyphs(GlyphMaker& gm, const std::wstring& text);

    float getLayoutBoundary() const;

    class Content : public RenderableBatch {
    public:
        Content();

        virtual bool preSnapshot(const RenderRequest& renderRequest, LayerContext& lc) override;
        virtual void snapshot(const RenderRequest& renderRequest, const LayerContext& lc, RenderLayerSnapshot& output) override;

        SafeVar<Vec3>& position();

        const std::vector<sp<RenderObject>>& renderObjects() const;
        void setRenderObjects(std::vector<sp<RenderObject>> renderObjects);

    private:
        SafeVar<Vec3> _position;
        std::vector<sp<RenderObject>> _render_objects;
        std::vector<Renderable::StateBits> _render_object_states;

        bool _needs_reload;
    };

private:
    BeanFactoryWeakRef _bean_factory;
    sp<RenderLayer> _render_layer;
    sp<LayerContext> _layer_context;
    sp<StringVar> _text;
    float _text_scale;
    sp<GlyphMaker> _glyph_maker;

    std::vector<sp<Glyph>> _glyphs;
    std::vector<sp<RenderablePassive>> _renderables;
    std::wstring _text_unicode;

    float _letter_spacing;
    float _layout_direction;
    float _line_height;
    float _line_indent;

    sp<ModelLoader> _model_loader;

    sp<Size> _size;
    sp<Size> _layout_size;

    sp<Content> _content;
};

}

#endif
