#ifndef ARK_GRAPHICS_IMPL_ALPHABET_TRUE_TYPE_ALPHABET_H_
#define ARK_GRAPHICS_IMPL_ALPHABET_TRUE_TYPE_ALPHABET_H_

#include <ft2build.h>
#include FT_FREETYPE_H

#include "core/base/string.h"
#include "core/inf/builder.h"

#include "graphics/base/font.h"
#include "graphics/inf/alphabet.h"
#include "graphics/util/freetypes.h"

namespace ark {

class AlphabetTrueType : public Alphabet {
public:
    AlphabetTrueType(const String& src, const Font::TextSize& textSize);
    ~AlphabetTrueType() override;

    virtual void setTextSize(const Font::TextSize& size);

    virtual Optional<Metrics> measure(int32_t c) override;
    virtual bool draw(uint32_t c, Bitmap& image, int32_t x, int32_t y) override;

//[[plugin::builder("true-type")]]
    class BUILDER : public Builder<Alphabet> {
    public:
        BUILDER(BeanFactory& factory, const document manifest);

        virtual sp<Alphabet> build(const Scope& args) override;

    private:
        sp<Builder<String>> _src;
        sp<Builder<String>> _text_size;
    };
private:
    void setFontSize(const Font::TextSize& textSize);
    sp<Readable> getFontResource(const String& path) const;

private:
    sp<FreeTypes> _free_types;
    Font::TextSize _text_size;

    FT_Face _ft_font_face;
    int32_t _base_line_position;
    int32_t _line_height_in_pixel;
};

}

#endif
