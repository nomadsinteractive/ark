#include "graphics/impl/character_maker/character_maker_text_color.h"

#include "core/base/bean_factory.h"
#include "core/inf/variable.h"

#include "graphics/base/render_object.h"


namespace ark {

CharacterMakerTextColor::CharacterMakerTextColor(sp<CharacterMaker> delegate, sp<Vec4> color)
    : _delegate(std::move(delegate)), _color(std::move(color))
{
}

std::vector<sp<RenderObject>> CharacterMakerTextColor::makeCharacter(const std::vector<Glyph>& glyphs)
{
    std::vector<sp<RenderObject>> renderObjects = _delegate->makeCharacter(glyphs);
    for(const sp<RenderObject>& i : renderObjects)
        i->varyings()->setProperty("Color", _color);
    return renderObjects;
}

V2 CharacterMakerTextColor::scale()
{
    return _delegate->scale();
}

CharacterMakerTextColor::BUILDER::BUILDER(BeanFactory& factory, const sp<Builder<CharacterMaker>>& delegate, const String& style)
    : _delegate(delegate), _color(factory.ensureBuilder<Vec4>(style))
{
}

sp<CharacterMaker> CharacterMakerTextColor::BUILDER::build(const Scope& args)
{
    return sp<CharacterMakerTextColor>::make(_delegate->build(args), _color->build(args));
}

}
