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

    Font(Family family = FONT_FAMILY_DEFAULT, Style style = FONT_STYLE_REGULAR, uint32_t size = 24);

    Family family() const;
    Style style() const;
    uint32_t size() const;

private:
    Family _family;
    Style _style;
    uint32_t _size;

};

}

#endif
