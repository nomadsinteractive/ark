#pragma once

#include "core/base/api.h"
#include "core/inf/builder.h"
#include "core/inf/wirable.h"
#include "core/impl/builder/safe_builder.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/base/layout_length.h"
#include "graphics/inf/render_batch.h"

namespace ark {

class ARK_API Text final : public Wirable {
public:
//  [[script::bindings::auto]]
    Text(sp<RenderLayer> renderLayer, sp<StringVar> text = nullptr, sp<Vec3> position = nullptr, sp<LayoutParam> layoutParam = nullptr, sp<Vec2> scale = nullptr, sp<GlyphMaker> glyphMaker = nullptr, float letterSpacing = 0.0f, LayoutLength lineHeight = {}, float lineIndent = 0.0f);

    void onWire(const WiringContext& context, const Box& self) override;

//  [[script::bindings::property]]
    const Vector<sp<RenderObject>>& contents() const;

//  [[script::bindings::property]]
    sp<Vec3> position() const;
//  [[script::bindings::property]]
    void setPosition(sp<Vec3> position);

//  [[script::bindings::property]]
    sp<Vec2> scale() const;
//  [[script::bindings::property]]
    void setScale(sp<Vec2> scale);

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
    void discard();

//  [[plugin::builder]]
    class BUILDER final : public Builder<Text> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        sp<Text> build(const Scope& args) override;

    private:
        sp<Builder<RenderLayer>> _render_layer;
        SafeBuilder<StringVar> _text;
        SafeBuilder<Font> _font;
        SafeBuilder<Vec3> _position;
        SafeBuilder<LayoutParam> _layout_param;
        SafeBuilder<Vec2> _scale;
        SafeBuilder<GlyphMaker> _glyph_maker;
        SafeBuilder<Numeric> _letter_spacing;
        SafeIBuilder<LayoutLength> _line_height;
        float _line_indent;
    };

//  [[plugin::builder("with-text")]]
    class BUILDER_WIRABLE final : public Builder<Wirable> {
    public:
        BUILDER_WIRABLE(BeanFactory& factory, const document& manifest);

        sp<Wirable> build(const Scope& args) override;

    private:
        builder<Text> _text;
    };

    struct Content;

private:
    sp<Content> _content;
    sp<RenderBatch> _render_batch;
};

}
