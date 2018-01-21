#ifndef ARK_GRAPHICS_IMPL_ALPHABET_TRUE_TYPE_ALPHABET_H_
#define ARK_GRAPHICS_IMPL_ALPHABET_TRUE_TYPE_ALPHABET_H_

#include <ft2build.h>
#include FT_FREETYPE_H

#include "core/base/string.h"
#include "core/inf/builder.h"

#include "graphics/inf/alphabet.h"

namespace ark {

class AlphabetTrueType : public Alphabet {
public:
    AlphabetTrueType(const String& src, uint32_t textSize);
    ~AlphabetTrueType();

    virtual bool load(uint32_t c, Metrics& metrics, bool loadGlyph, bool hasFallback) override;
    virtual void draw(const bitmap& image, int32_t x, int32_t y) override;

//[[plugin::builder("true-type")]]
    class BUILDER : public Builder<Alphabet> {
    public:
        BUILDER(BeanFactory& factory, const document manifest);

        virtual sp<Alphabet> build(const sp<Scope>& args) override;

    private:
        String _src;
        uint32_t _text_size;
    };
private:
    sp<Readable> getFontResource(const String& path) const;

private:
    FT_Face _ft_font_face;
    int32_t _base_line_position;
    uint32_t _line_height_in_pixel;

};

}

#endif
