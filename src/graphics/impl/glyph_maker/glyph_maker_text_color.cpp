#include "graphics/impl/glyph_maker/glyph_maker_text_color.h"

#include "core/inf/variable.h"

#include "graphics/base/glyph.h"

#include "renderer/base/varyings.h"

namespace ark {

CharacterMakerTextColor::CharacterMakerTextColor(sp<GlyphMaker> delegate, sp<Vec4> color)
    : _delegate(std::move(delegate)), _varyings(sp<Varyings>::make()), _color(std::move(color))
{
    _varyings->setProperty("Color", _color);
}

std::vector<sp<Glyph>> CharacterMakerTextColor::makeGlyphs(const std::wstring& text)
{
    std::vector<sp<Glyph>> glyphs = _delegate->makeGlyphs(text);
    for(const sp<Glyph>& i : glyphs)
        if(i->varyings())
            i->varyings()->setProperty("Color", _color);
        else
            i->setVaryings(_varyings);
    return glyphs;
}

}
