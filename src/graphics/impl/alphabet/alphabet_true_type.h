#ifndef ARK_GRAPHICS_IMPL_ALPHABET_TRUE_TYPE_ALPHABET_H_
#define ARK_GRAPHICS_IMPL_ALPHABET_TRUE_TYPE_ALPHABET_H_

#include <mutex>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "core/base/string.h"
#include "core/inf/builder.h"

#include "graphics/inf/alphabet.h"
#include "graphics/util/freetypes.h"

namespace ark {

class AlphabetTrueType : public Alphabet {
public:
    struct TextSize {
        enum Unit {
            TEXT_SIZE_UNIT_PX = 0,
            TEXT_SIZE_UNIT_PT = 1
        };

        TextSize(const String& size);
        TextSize(uint32_t value, Unit unit);

        uint32_t _value;
        Unit _unit;
    };

public:
    AlphabetTrueType(const String& src, const TextSize& textSize);
    ~AlphabetTrueType() override;

    virtual bool measure(int32_t c, Metrics& metrics, bool hasFallback) override;
    virtual bool draw(uint32_t c, const bitmap& image, int32_t x, int32_t y) override;

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
    sp<Readable> getFontResource(const String& path) const;

private:
    sp<FreeTypes> _free_types;

    FT_Face _ft_font_face;
    int32_t _base_line_position;
    int32_t _line_height_in_pixel;

    std::mutex _mutex;

};

}

#endif
