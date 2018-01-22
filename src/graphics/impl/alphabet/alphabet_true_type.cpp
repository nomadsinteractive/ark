#include "graphics/impl/alphabet/alphabet_true_type.h"

#include "core/inf/dictionary.h"
#include "core/impl/readable/file_readable.h"
#include "core/util/documents.h"
#include "core/types/global.h"

#include "graphics/base/bitmap.h"
#include "graphics/util/freetypes.h"

#include "platform/platform.h"

namespace ark {

AlphabetTrueType::AlphabetTrueType(const String& src, uint32_t textSize)
{
    const Global<FreeTypes> freetypes;
    const sp<Readable> readable = getFontResource(src);
    DCHECK(readable, "Font \"%s\" does not exists", src.c_str());
    freetypes->ftNewFaceFromReadable(readable, 0, &_ft_font_face);
    FT_Set_Char_Size(_ft_font_face, FreeTypes::ftF26Dot6(textSize, 0), 0, 96, 0);
    _line_height_in_pixel = FreeTypes::ftCalculateLineHeight(_ft_font_face);
    _base_line_position = _line_height_in_pixel + FreeTypes::ftCalculateBaseLinePosition(_ft_font_face);
}

AlphabetTrueType::~AlphabetTrueType()
{
    FT_Done_Face(_ft_font_face);
}

bool AlphabetTrueType::measure(uint32_t c, Metrics& metrics, bool hasFallback)
{
    FT_UInt glyphIndex = FT_Get_Char_Index(_ft_font_face, c);
    if(hasFallback && !glyphIndex)
        return false;
    if(FT_Load_Glyph(_ft_font_face, glyphIndex, FT_LOAD_NO_BITMAP) != 0)
        DFATAL("Error loading metrics, character: %d", c);
    FT_GlyphSlot slot = _ft_font_face->glyph;
    metrics.width = slot->advance.x >> 6;
    metrics.height = _line_height_in_pixel;
    metrics.bitmap_width = slot->metrics.width >> 6;
    metrics.bitmap_height = slot->metrics.height >> 6;
    metrics.bitmap_x = slot->metrics.horiBearingX >> 6;
    metrics.bitmap_y = _base_line_position - (slot->metrics.horiBearingY >> 6);
    return true;
}

bool AlphabetTrueType::draw(uint32_t c, const bitmap& image, int32_t x, int32_t y)
{
    FT_UInt glyphIndex = FT_Get_Char_Index(_ft_font_face, c);
    if(!glyphIndex)
        return false;
    if(FT_Load_Glyph(_ft_font_face, glyphIndex, FT_LOAD_RENDER) != 0)
        DFATAL("Error loading glyph, character: %d", c);
    FT_GlyphSlot slot = _ft_font_face->glyph;
    DCHECK(slot, "Glyph not loaded");
//    image->draw(slot->bitmap.buffer, slot->bitmap.width, slot->bitmap.rows, x + slot->bitmap_left, y + _base_line_position - slot->bitmap_top, slot->bitmap.pitch);
    image->draw(slot->bitmap.buffer, slot->bitmap.width, slot->bitmap.rows, x, y, slot->bitmap.pitch);
    return true;
}

sp<Readable> AlphabetTrueType::getFontResource(const String& name) const
{
    const String path = Platform::pathJoin(Platform::getDefaultFontDirectory(), name);
    if(Platform::isFile(path))
        return sp<FileReadable>::make(path, "rb");
    return Ark::instance().getResource(name);
}

AlphabetTrueType::BUILDER::BUILDER(BeanFactory& /*factory*/, const document manifest)
    : _src(Documents::getAttribute(manifest, Constants::Attributes::SRC)),
      _text_size(Documents::getAttribute<uint32_t>(manifest, Constants::Attributes::TEXT_SIZE, 24))
{
}

sp<Alphabet> AlphabetTrueType::BUILDER::build(const sp<Scope>& args)
{
    return sp<AlphabetTrueType>::make(_src, _text_size);
}

}
