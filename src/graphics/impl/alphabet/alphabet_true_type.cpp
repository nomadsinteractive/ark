#include "graphics/impl/alphabet/alphabet_true_type.h"

#include "core/ark.h"
#include "core/base/bean_factory.h"
#include "core/impl/readable/file_readable.h"
#include "core/util/documents.h"

#include "graphics/base/bitmap.h"

#include "platform/platform.h"

namespace ark {

AlphabetTrueType::AlphabetTrueType(const String& src, const Font::TextSize& textSize)
    : _free_types(Ark::instance().ensure<FreeTypes>()), _text_size(textSize)
{
    sp<Readable> readable = getFontResource(src);
    CHECK(readable, "Font \"%s\" does not exists", src.c_str());
    _free_types->ftNewFaceFromReadable(std::move(readable), 0, &_ft_font_face);
    setFontSize(textSize);
}

AlphabetTrueType::~AlphabetTrueType()
{
    FT_Done_Face(_ft_font_face);
}

void AlphabetTrueType::setTextSize(const Font::TextSize& size)
{
    setFontSize(size._value > 0 ? size : _text_size);
}

Optional<Alphabet::Metrics> AlphabetTrueType::measure(int32_t c)
{
    FT_UInt glyphIndex = FT_Get_Char_Index(_ft_font_face, c);
    if(!glyphIndex)
        return Optional<Metrics>();

    FT_Error err;
    if((err = FT_Load_Glyph(_ft_font_face, glyphIndex, FT_LOAD_NO_BITMAP)) != 0)
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
    if(FT_Load_Glyph(_ft_font_face, glyphIndex, FT_LOAD_RENDER | FT_LOAD_TARGET_NORMAL) != 0)
        DFATAL("Error loading glyph, character: %d", c);
    FT_GlyphSlot slot = _ft_font_face->glyph;
    DCHECK(slot, "Glyph not loaded");
    image.draw(x, y, slot->bitmap.buffer, slot->bitmap.width, slot->bitmap.rows, static_cast<uint32_t>(slot->bitmap.pitch));
    return true;
}

void ark::AlphabetTrueType::setFontSize(const Font::TextSize& textSize)
{
    if(textSize._unit == Font::FONT_SIZE_UNIT_PT)
        FT_Set_Char_Size(_ft_font_face, FreeTypes::ftF26Dot6(textSize._value, 0), 0, 96, 0);
    else
    {
        DASSERT(textSize._unit == Font::FONT_SIZE_UNIT_PX);
        FT_Set_Pixel_Sizes(_ft_font_face, textSize._value, textSize._value);
    }
    _line_height_in_pixel = FreeTypes::ftCalculateLineHeight(_ft_font_face);
    _base_line_position = _line_height_in_pixel + FreeTypes::ftCalculateBaseLinePosition(_ft_font_face);
}

sp<Readable> AlphabetTrueType::getFontResource(const String& name) const
{
    const String path = Platform::pathJoin(Platform::getDefaultFontDirectory(), name);
    if(Platform::isFile(path))
        return sp<FileReadable>::make(path, "rb");
    return Ark::instance().tryOpenAsset(name);
}

AlphabetTrueType::BUILDER::BUILDER(BeanFactory& factory, const document manifest)
    : _src(factory.ensureBuilder<String>(manifest, Constants::Attributes::SRC)),
      _text_size(factory.ensureBuilder<String>(manifest, Constants::Attributes::TEXT_SIZE))
{
}

sp<Alphabet> AlphabetTrueType::BUILDER::build(const Scope& args)
{
    return sp<AlphabetTrueType>::make(_src->build(args), Font::TextSize(_text_size->build(args)));
}

}
