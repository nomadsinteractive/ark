#include "graphics/impl/alphabet/alphabet_true_type.h"

#include "core/base/bean_factory.h"
#include "core/base/constants.h"
#include "core/impl/readable/file_readable.h"
#include "core/types/global.h"

#include "graphics/base/bitmap.h"

#include "platform/platform.h"

namespace ark {

namespace {

sp<Readable> getFontResource(const String& name)
{
    if(const String path = Platform::pathJoin(Platform::getDefaultFontDirectory(), name); Platform::isFile(path))
        return sp<Readable>::make<FileReadable>(path, "rb");
    return Ark::instance().tryOpenAsset(name);
}

}

AlphabetTrueType::AlphabetTrueType(const String& src)
    : _free_types(Global<FreeTypes>()), _font(Font::TextSize())
{
    sp<Readable> readable = getFontResource(src);
    CHECK(readable, "Font \"%s\" does not exists", src.c_str());
    _free_types->ftNewFaceFromReadable(std::move(readable), 0, &_ft_font_face);
}

AlphabetTrueType::~AlphabetTrueType()
{
    FT_Done_Face(_ft_font_face);
}

void AlphabetTrueType::setFont(const Font& font)
{
    ASSERT(font);
    _font = font;

    if(const Font::TextSize& textSize = font.size(); textSize._unit == Font::FONT_SIZE_UNIT_PT)
        FT_Set_Char_Size(_ft_font_face, FreeTypes::ftF26Dot6(textSize._value, 0), 0, 96, 0);
    else
    {
        DASSERT(textSize._unit == Font::FONT_SIZE_UNIT_PX);
        FT_Set_Pixel_Sizes(_ft_font_face, textSize._value, textSize._value);
    }
    _line_height_in_pixel = FreeTypes::ftCalculateLineHeight(_ft_font_face);
    _base_line_position = _line_height_in_pixel + FreeTypes::ftCalculateBaseLinePosition(_ft_font_face);
}

Optional<Alphabet::Metrics> AlphabetTrueType::measure(int32_t c)
{
    const FT_UInt glyphIndex = FT_Get_Char_Index(_ft_font_face, c);
    if(!glyphIndex)
        return {};

    if(const FT_Error err = FT_Load_Glyph(_ft_font_face, glyphIndex, FT_LOAD_NO_BITMAP); err != 0)
        CHECK_WARN(false, "Error loading metrics, character: %d. Error code: %d", c, err);

    FT_GlyphSlot slot = _ft_font_face->glyph;
    Alphabet::Metrics metrics;
    metrics.width = slot->advance.x >> 6;
    metrics.height = _line_height_in_pixel;
    metrics.bitmap_width = slot->metrics.width >> 6;
    metrics.bitmap_height = slot->metrics.height >> 6;
    metrics.bitmap_x = slot->metrics.horiBearingX >> 6;
    metrics.bitmap_y = _base_line_position - (slot->metrics.horiBearingY >> 6);
    return metrics;
}

bool AlphabetTrueType::draw(uint32_t c, Bitmap& image, int32_t x, int32_t y)
{
    FT_UInt glyphIndex = FT_Get_Char_Index(_ft_font_face, c);
    if(!glyphIndex)
        return false;

    const bool isMonochrome = _font.style() & Font::FONT_STYLE_MONOCHROME;
    if(FT_Load_Glyph(_ft_font_face, glyphIndex, FT_LOAD_RENDER | (isMonochrome ? FT_LOAD_TARGET_MONO : FT_LOAD_TARGET_NORMAL)) != 0)
        DFATAL("Error loading glyph, character: %d", c);
    FT_GlyphSlot slot = _ft_font_face->glyph;
    DCHECK(slot, "Glyph not loaded");

    if(isMonochrome)
    {
        Vector<uint8_t> pixels(slot->bitmap.width);
        for(uint32_t i = 0; i < slot->bitmap.rows; ++i)
        {
            const uint8_t* rowbuf = slot->bitmap.buffer + slot->bitmap.pitch * i;
            for(uint32_t j = 0; j < slot->bitmap.width; ++j)
            {
                const uint8_t pv = rowbuf[j / sizeof(uint8_t)];
                pixels[j] = pv & (1 << (7 - (j % sizeof(uint8_t)))) ? std::numeric_limits<uint8_t>::max() : 0;
            }
            image.draw(x, y + i, pixels.data(), slot->bitmap.width, 1, pixels.size());
        }
    }
    else
        image.draw(x, y, slot->bitmap.buffer, slot->bitmap.width, slot->bitmap.rows, static_cast<uint32_t>(slot->bitmap.pitch));
    return true;
}

AlphabetTrueType::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _src(factory.ensureBuilder<String>(manifest, constants::SRC))
{
}

sp<Alphabet> AlphabetTrueType::BUILDER::build(const Scope& args)
{
    return sp<Alphabet>::make<AlphabetTrueType>(_src->build(args));
}

}
