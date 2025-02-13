#pragma once

#include <ft2build.h>
#include FT_FREETYPE_H

#include "core/base/string.h"
#include "core/inf/builder.h"

#include "graphics/base/font.h"
#include "graphics/inf/alphabet.h"
#include "graphics/util/freetypes.h"

namespace ark {

class AlphabetTrueType final : public Alphabet {
public:
    AlphabetTrueType(const String& src);
    ~AlphabetTrueType() override;

    void setFont(const Font& font) override;

    Optional<Metrics> measure(int32_t c) override;
    bool draw(uint32_t c, Bitmap& image, int32_t x, int32_t y) override;

//  [[plugin::builder("true-type")]]
    class BUILDER final: public Builder<Alphabet> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        sp<Alphabet> build(const Scope& args) override;

    private:
        sp<Builder<String>> _src;
    };

private:
    sp<FreeTypes> _free_types;
    Font _font;

    FT_Face _ft_font_face;
    int32_t _base_line_position;
    int32_t _line_height_in_pixel;
};

}
