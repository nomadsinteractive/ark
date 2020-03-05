#include "graphics/impl/character_maker/character_maker_text_color.h"

#include "core/base/bean_factory.h"
#include "core/inf/variable.h"

#include "graphics/base/render_object.h"


namespace ark {

CharacterMakerTextColor::CharacterMakerTextColor(sp<CharacterMaker> delegate, sp<Vec4> color)
    : _delegate(std::move(delegate)), _color(std::move(color))
{
}

sp<RenderObject> CharacterMakerTextColor::makeCharacter(int32_t type, const V3& position, const sp<Size>& size)
{
    const sp<RenderObject> renderObject = sp<RenderObject>::make(type, sp<Vec3>::make<Vec3::Const>(position), size);
    renderObject->varyings()->set("Color", _color);
    return renderObject;
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
