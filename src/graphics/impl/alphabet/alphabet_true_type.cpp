#include "graphics/impl/alphabet/alphabet_true_type.h"

#include "core/ark.h"
#include "core/base/bean_factory.h"
#include "core/impl/readable/file_readable.h"
#include "core/util/documents.h"

#include "graphics/base/bitmap.h"

#include "platform/platform.h"

namespace ark {

AlphabetTrueType::TextSize::TextSize(const String& size)
{
    _unit = TEXT_SIZE_UNIT_PT;

    if(size.endsWith("px"))
    {
        _unit = TEXT_SIZE_UNIT_PX;
        _value = Strings::parse<uint32_t>(size.substr(0, size.length() - 2));
    }
    else if(size.endsWith("pt"))
        _value = Strings::parse<uint32_t>(size.substr(0, size.length() - 2));
    else
        _value = Strings::parse<uint32_t>(size);
}

AlphabetTrueType::TextSize::TextSize(uint32_t value, AlphabetTrueType::TextSize::Unit unit)
    : _value(value), _unit(unit)
{
}

AlphabetTrueType::AlphabetTrueType(const String& src, const TextSize& textSize)
    : _free_types(Ark::instance().ensure<FreeTypes>())
{
    const sp<Readable> readable = getFontResource(src);
    DCHECK(readable, "Font \"%s\" does not exists", src.c_str());
    _free_types->ftNewFaceFromReadable(readable, 0, &_ft_font_face);
    if(textSize._unit == TextSize::TEXT_SIZE_UNIT_PT)
        FT_Set_Char_Size(_ft_font_face, FreeTypes::ftF26Dot6(textSize._value, 0), 0, 96, 0);
    else
    {
        DASSERT(textSize._unit == TextSize::TEXT_SIZE_UNIT_PX);
        FT_Set_Pixel_Sizes(_ft_font_face, textSize._value, textSize._value);
    }
    _line_height_in_pixel = FreeTypes::ftCalculateLineHeight(_ft_font_face);
    _base_line_position = _line_height_in_pixel + FreeTypes::ftCalculateBaseLinePosition(_ft_font_face);
}

AlphabetTrueType::~AlphabetTrueType()
{
    FT_Done_Face(_ft_font_face);
}

bool AlphabetTrueType::measure(int32_t c, Metrics& metrics, bool hasFallback)
{
    const auto iter = _metrics_cache.find(c);
    if(iter != _metrics_cache.end())
    {
        metrics = iter->second;
        return true;
    }

    FT_UInt glyphIndex = FT_Get_Char_Index(_ft_font_face, c);
    if(hasFallback && !glyphIndex)
        return false;

    FT_Error err;
    if((err = FT_Load_Glyph(_ft_font_face, glyphIndex, FT_LOAD_NO_BITMAP)) != 0)
        DFATAL("Error loading metrics, character: %d. Error code: %d", c, err);
    FT_GlyphSlot slot = _ft_font_face->glyph;
    metrics.width = slot->advance.x >> 6;
    metrics.height = _line_height_in_pixel;
    metrics.bitmap_width = slot->metrics.width >> 6;
    metrics.bitmap_height = slot->metrics.height >> 6;
    metrics.bitmap_x = slot->metrics.horiBearingX >> 6;
    metrics.bitmap_y = _base_line_position - (slot->metrics.horiBearingY >> 6);
    _metrics_cache[c] = metrics;
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
    image->draw(x, y, slot->bitmap.buffer, slot->bitmap.width, slot->bitmap.rows, static_cast<uint32_t>(slot->bitmap.pitch));
    return true;
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
    return sp<AlphabetTrueType>::make(_src->build(args), TextSize(_text_size->build(args)));
}

}
