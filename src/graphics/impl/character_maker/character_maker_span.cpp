#include "graphics/impl/character_maker/character_maker_span.h"

#include "core/inf/variable.h"
#include "core/util/documents.h"

#include "graphics/base/render_object.h"


namespace ark {

CharacterMakerSpan::CharacterMakerSpan(const V2& scale)
    : _scale(scale)
{
}

CharacterMakerSpan::CharacterMakerSpan(const V2& scale, BeanFactory& factory, const sp<String>& className)
    : _scale(scale), _character_builder(className ? factory.createBuilderByRef<RenderObject>(Identifier::parseRef(*className)) : sp<Builder<RenderObject>>::null())
{
    DCHECK(!className || _character_builder, "Cann not build RenderObject, class: \"%s\"", className->c_str());
}

sp<RenderObject> CharacterMakerSpan::makeCharacter(int32_t type, const V3& position, const sp<Size>& size)
{
    if(_character_builder)
    {
        const sp<RenderObject> renderObject = _character_builder->build(Scope());
        renderObject->setType(type);
        renderObject->setPosition(sp<Vec3::Const>::make(position));
        renderObject->setSize(size);
        return renderObject;
    }
    return sp<RenderObject>::make(type, sp<Vec3>::make<Vec3::Const>(position), size);
}

V2 CharacterMakerSpan::scale()
{
    return _scale;
}

CharacterMakerSpan::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _bean_factory(factory), _class_name(factory.getBuilder<String>(manifest, Constants::Attributes::CLASS))
{
}

sp<CharacterMaker> CharacterMakerSpan::BUILDER::build(const Scope& args)
{
    return sp<CharacterMakerSpan>::make(V2(1.0f), _bean_factory, _class_name->build(args));
}

}