#ifndef ARK_RENDERER_BASE_CHARACTERS_H_
#define ARK_RENDERER_BASE_CHARACTERS_H_

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

#include "renderer/forwarding.h"

#include "app/forwarding.h"

namespace ark {

class ARK_API Characters {
public:
//  [[script::bindings::auto]]
    Characters(const sp<Layer>& layer, float textScale = 1.0f, float letterSpacing = 0.0f, float lineHeight = 0.0f, float lineIndent = 0.0f);
//  [[script::bindings::auto]]
    Characters(const sp<RenderLayer>& layer, float textScale = 1.0f, float letterSpacing = 0.0f, float lineHeight = 0.0f, float lineIndent = 0.0f);
//  [[script::bindings::auto]]
    Characters(const sp<LayerContext>& layer, float textScale = 1.0f, float letterSpacing = 0.0f, float lineHeight = 0.0f, float lineIndent = 0.0f);

    Characters(const BeanFactory& factory, const sp<LayerContext>& layerContext, const sp<CharacterMapper>& characterMapper, const sp<CharacterMaker>& characterMaker, float textScale, float letterSpacing, float lineHeight, float lineIndent);

    const sp<LayoutParam>& layoutParam() const;
    void setLayoutParam(const sp<LayoutParam>& layoutParam);

//  [[script::bindings::property]]
    const std::vector<sp<RenderObject>>& contents() const;

//  [[script::bindings::property]]
    const SafePtr<Size>& size() const;

//  [[script::bindings::property]]
    const std::wstring& text() const;
//  [[script::bindings::property]]
    void setText(const std::wstring& text);

//  [[script::bindings::auto]]
    void setRichText(const std::wstring& richText, const Scope& args);

    void renderRequest(const V3& position);

//[[plugin::builder]]
    class BUILDER : public Builder<Characters> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        virtual sp<Characters> build(const Scope& args) override;

    private:
        BeanFactory _bean_factory;
        sp<Builder<LayerContext>> _layer_context;
        SafePtr<Builder<CharacterMapper>> _character_mapper;
        SafePtr<Builder<CharacterMaker>> _character_maker;

        sp<Builder<String>> _text_scale;
        float _letter_spacing;
        float _line_height;
        float _line_indent;
    };

private:
//    typedef std::function<sp<RenderObject>(int32_t, const V3&, float, float)> ContentMaker;
    typedef std::vector<Glyph> ContentMaker;

//    class CharacterContentMaker {
//    public:
//        CharacterContentMaker(sp<CharacterMaker> characterMaker);

//        sp<RenderObject> operator() (int32_t type, const V3&position, float width, float height);

//    private:
//        sp<CharacterMaker> _character_maker;
//    };

//    class RelayoutContentMaker {
//    public:
//        RelayoutContentMaker(std::list<sp<RenderObject>> characters);

//        sp<RenderObject> operator() (int32_t type, const V3& position, float width, float height);

//    private:
//        std::list<sp<RenderObject>> _characters;
//    };

    void createContent();
    void createRichContent(const Scope& args);
    float createContentWithBoundary(ContentMaker& cm, const V2& s, float& flowx, float& flowy, const std::wstring& text, float boundary);
    float createContentNoBoundary(ContentMaker& cm, const V2& s, float& flowx, float flowy, const std::wstring& text);

    float doCreateContent(ContentMaker& cm, const V2& s, float& flowx, float& flowy, const std::wstring& text, float boundary);
    float doCreateRichContent(ContentMaker& cm, const V2& s, const document& richtext, BeanFactory& factory, const Scope& args, float& flowx, float& flowy, float boundary);
    void doLayoutContent();

    void createLayerContent(float width, float height);


    struct LayoutChar {
        LayoutChar(int32_t type, const Metrics& metrics, float widthIntegral, bool isCJK, bool isWordBreak, bool isLineBreak);

        int32_t _type;
        Metrics _metrics;
        float _width_integral;
        bool _is_cjk;
        bool _is_word_break;
        bool _is_line_break;
    };

    void place(ContentMaker& cm, const V2& s, const std::vector<LayoutChar>& layouts, size_t begin, size_t end, float& flowx, float flowy);
    void placeOne(ContentMaker& cm, const V2& s, const Metrics& metrics, int32_t type, float& flowx, float flowy, float* fontHeight = nullptr);

    void nextLine(float fontHeight, float& flowx, float& flowy) const;
    float getFlowY() const;

    std::vector<LayoutChar> getCharacterMetrics(const V2& s, const std::wstring& text) const;
    bool isCJK(int32_t c) const;
    bool isWordBreaker(wchar_t c) const;

    int32_t toType(wchar_t c) const;

private:
    BeanFactoryWeakRef _bean_factory;
    sp<LayerContext> _layer_context;
    float _text_scale;
    sp<LayoutParam> _layout_param;
    sp<CharacterMapper> _character_mapper;
    sp<CharacterMaker> _character_maker;

    std::vector<sp<RenderObject>> _contents;
    std::vector<sp<RenderablePassive>> _renderables;
    std::wstring _text;

    float _letter_spacing;
    float _layout_direction;
    float _line_height;
    float _line_indent;

    sp<ModelLoader> _model_loader;

    SafePtr<Size> _size;
    V3 _layout_size;
};

}

#endif
