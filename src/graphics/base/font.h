#pragma once

#include "core/base/api.h"
#include "core/base/string.h"
#include "core/inf/builder.h"

namespace ark {

class ARK_API Font {
public:
//  [[script::bindings::enumeration]]
    enum Style {
        FONT_STYLE_REGULAR = 0,
        FONT_STYLE_BOLD = 1,
        FONT_STYLE_ITALIC = 2,
        FONT_STYLE_MONOCHROME = 4
    };

//  [[script::bindings::enumeration]]
    enum SizeUnit {
        FONT_SIZE_UNIT_PX = 0,
        FONT_SIZE_UNIT_PT = 1
    };

    struct TextSize {
        TextSize();
        TextSize(const String& size);
        TextSize(uint32_t value, SizeUnit unit);

        uint32_t _value;
        SizeUnit _unit;
    };

//  [[script::bindings::auto]]
    Font(const String& size, Font::Style style = Font::FONT_STYLE_REGULAR);
    Font(TextSize size, Style style = FONT_STYLE_REGULAR);
    Font(uint32_t size, SizeUnit sizeUnit, Style style = FONT_STYLE_REGULAR);

    explicit operator bool() const;

    TextSize size() const;

    uint32_t sizeValue() const;
    SizeUnit sizeUnit() const;

//  [[script::bindings::property]]
    Font::Style style() const;

    uint32_t combine(uint32_t unicode) const;

    bool operator < (const Font& other) const;

    static std::pair<Font, uint32_t> partition(uint32_t combined);

//  [[plugin::builder]]
    class BUILDER final : public Builder<Font> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        sp<Font> build(const Scope& args) override;

    private:
        builder<String> _text_size;
    };

//  [[plugin::builder::by-value]]
    class DICTIONARY final : public Builder<Font> {
    public:
        DICTIONARY(BeanFactory& beanFactory, const String& expr);

        sp<Font> build(const Scope& args) override;

    private:
        builder<String> _text_size;
    };

private:
    TextSize _size;
    Style _style;
};

}
