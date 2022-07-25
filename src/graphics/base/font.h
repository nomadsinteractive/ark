#ifndef ARK_GRAPHICS_BASE_FONT_H_
#define ARK_GRAPHICS_BASE_FONT_H_

#include "core/base/api.h"

namespace ark {

class Font {
public:
    enum Family {
        FONT_FAMILY_DEFAULT,
        FONT_FAMILY_ARIAL,
        FONT_FAMILY_MONOSPACE,
        FONT_FAMILY_SANS_SERIF,
        FONT_FAMILY_SERIF
    };

    enum Style {
        FONT_STYLE_REGULAR = 0,
        FONT_STYLE_BOLD = 1,
        FONT_STYLE_ITALIC = 2
    };

    enum SizeUnit {
        FONT_SIZE_UNIT_PX = 0,
        FONT_SIZE_UNIT_PT = 1
    };

    struct TextSize {
        TextSize();
        TextSize(const String& size);
        TextSize(uint32_t value, SizeUnit unit);

        bool operator < (const TextSize& other) const;

        uint32_t _value;
        SizeUnit _unit;
    };

    Font(const TextSize& size, Family family = FONT_FAMILY_DEFAULT, Style style = FONT_STYLE_REGULAR);

    const TextSize& size() const;
    Family family() const;
    Style style() const;

    bool operator < (const Font& other) const;

private:
    TextSize _size;
    Family _family;
    Style _style;

};

}

#endif
