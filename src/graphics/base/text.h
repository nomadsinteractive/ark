#pragma once

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
    Text(sp<RenderLayer> renderLayer, sp<StringVar> content = nullptr, sp<LayoutParam> layoutParam = nullptr, sp<GlyphMaker> glyphMaker = nullptr, float letterScale = 1.0f, float letterSpacing = 0.0f, float lineHeight = 0.0f, float lineIndent = 0.0f);

//  [[script::bindings::property]]
    const std::vector<sp<RenderObject>>& contents() const;

//  [[script::bindings::property]]
    const sp<LayoutParam>& layoutParam() const;
//  [[script::bindings::property]]
    void setLayoutParam(sp<LayoutParam> layoutParam);

//  [[script::bindings::property]]
    sp<Vec3> position() const;
//  [[script::bindings::property]]
    void setPosition(sp<Vec3> position);

//  [[script::bindings::property]]
    const sp<Size>& size() const;

//  [[script::bindings::property]]
    const sp<Boundaries>& boundaries() const;
//  [[script::bindings::property]]
    void setBoundaries(sp<Boundaries> boundaries);

//  [[script::bindings::property]]
    const std::wstring& text() const;
//  [[script::bindings::property]]
    void setText(std::wstring text);

//  [[script::bindings::auto]]
    void show(sp<Boolean> discarded = nullptr);
//  [[script::bindings::auto]]
    void hide();

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
        SafePtr<Builder<LayoutParam>> _layout_param;
        SafePtr<Builder<GlyphMaker>> _glyph_maker;

        sp<Builder<String>> _text_scale;
        sp<Builder<Numeric>> _letter_spacing;
        float _line_height;
        float _line_indent;
    };

    struct Content;

private:
    sp<RenderLayer> _render_layer;
    sp<Content> _content;
    sp<RenderBatch> _render_batch;
};

}
