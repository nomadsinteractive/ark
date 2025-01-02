#include "graphics/impl/glyph_maker/glyph_maker_text_color.h"

#include "core/base/bean_factory.h"
#include "core/inf/variable.h"

#include "graphics/components/render_object.h"
#include "graphics/base/glyph.h"

namespace ark {

CharacterMakerTextColor::CharacterMakerTextColor(sp<GlyphMaker> delegate, sp<Vec4> color)
    : _delegate(std::move(delegate)), _varyings(sp<Varyings>::make()), _color(std::move(color))
{
    _varyings->setProperty("Color", _color);
}

std::vector<sp<Glyph>> CharacterMakerTextColor::makeGlyphs(const std::wstring& text)
{
    std::vector<sp<Glyph>> renderObjects = _delegate->makeGlyphs(text);
    for(const sp<Glyph>& i : renderObjects)
        if(i->varyings())
            i->varyings()->setProperty("Color", _color);
        else
            i->setVaryings(_varyings);
    return renderObjects;
}

CharacterMakerTextColor::BUILDER::BUILDER(BeanFactory& factory, const sp<Builder<GlyphMaker>>& delegate, const String& style)
    : _delegate(delegate), _color(factory.ensureBuilder<Vec4>(style))
{
}

sp<GlyphMaker> CharacterMakerTextColor::BUILDER::build(const Scope& args)
{
    return sp<CharacterMakerTextColor>::make(_delegate->build(args), _color->build(args));
}

}
