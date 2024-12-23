#pragma once

#include <vector>

#include "core/base/api.h"
#include "core/base/bean_factory.h"
#include "core/inf/builder.h"
#include "core/types/safe_builder.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/inf/renderable.h"
#include "graphics/inf/render_batch.h"

namespace ark {

class ARK_API Text {
public:
//  [[script::bindings::auto]]
    Text(sp<RenderLayer> renderLayer, sp<StringVar> text = nullptr, sp<Vec3> position = nullptr, sp<LayoutParam> layoutParam = nullptr, sp<GlyphMaker> glyphMaker = nullptr, sp<Mat4> transform = nullptr, float letterSpacing = 0.0f, float lineHeight = 0.0f, float lineIndent = 0.0f);

//  [[script::bindings::property]]
    const std::vector<sp<RenderObject>>& contents() const;

//  [[script::bindings::property]]
    const sp<LayoutParam>& layoutParam() const;
//  [[script::bindings::property]]
    void setLayoutParam(sp<LayoutParam> layoutParam);

//  [[script::bindings::property]]
    const SafeVar<Vec3>& position() const;
//  [[script::bindings::property]]
    void setPosition(sp<Vec3> position);

//  [[script::bindings::property]]
    const sp<Mat4>& transform() const;
//  [[script::bindings::property]]
    void setTransform(sp<Mat4> transform);

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

//  [[plugin::builder]]
    class BUILDER final : public Builder<Text> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        sp<Text> build(const Scope& args) override;

    private:
        sp<Builder<RenderLayer>> _render_layer;
        SafeBuilder<StringVar> _text;
        SafeBuilder<Vec3> _position;
        SafeBuilder<LayoutParam> _layout_param;
        SafeBuilder<GlyphMaker> _glyph_maker;
        SafeBuilder<Mat4> _transform;
        SafeBuilder<Numeric> _letter_spacing;
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
