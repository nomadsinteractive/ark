#ifndef ARK_RENDERER_BASE_CHARACTERS_H_
#define ARK_RENDERER_BASE_CHARACTERS_H_

#include <string>
#include <vector>

#include "core/base/api.h"
#include "core/base/bean_factory.h"
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

        float _text_scale;
        float _letter_spacing;
        float _line_height;
        float _line_indent;
    };

private:
    void createContent();
    void createRichContent(const Scope& args);
    float createContentWithBoundary(CharacterMaker& cm, float& flowx, float& flowy, const std::wstring& text, float boundary);
    float createContentNoBoundary(CharacterMaker& cm, float& flowx, float flowy, const std::wstring& text);

    float doCreateContent(CharacterMaker& cm, float& flowx, float& flowy, const std::wstring& text, float boundary);
    float doCreateRichContent(CharacterMaker& cm, const document& richtext, BeanFactory& factory, const Scope& args, float& flowx, float& flowy, float boundary);

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

    void place(CharacterMaker& cm, const std::vector<LayoutChar>& layouts, size_t begin, size_t end, float& flowx, float flowy);
    void placeOne(CharacterMaker& cm, const Metrics& metrics, int32_t type, float& flowx, float flowy, float* fontHeight = nullptr);

    void nextLine(float fontHeight, float& flowx, float& flowy) const;
    float getFlowY() const;

    std::vector<LayoutChar> getCharacterMetrics(CharacterMaker& cm, const std::wstring& text) const;
    bool isCJK(int32_t c) const;
    bool isWordBreaker(wchar_t c) const;

    int32_t toType(wchar_t c) const;
    sp<RenderObject> makeCharacter(int32_t type, const V3& position, const sp<Size>& size) const;

private:
    BeanFactory::WeakRef _bean_factory;
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
