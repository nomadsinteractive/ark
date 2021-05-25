#include "graphics/impl/character_maker/character_maker_span.h"

#include "core/inf/variable.h"
#include "core/util/documents.h"

#include "graphics/base/glyph.h"
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

std::vector<sp<RenderObject>> CharacterMakerSpan::makeCharacter(const std::vector<Glyph>& glyphs)
{
    const Scope scope;
    std::vector<sp<RenderObject>> renderObjects;
    for(const Glyph& i : glyphs)
    {
        sp<Size> size = sp<Size>::make(i.size().x(), i.size().y());
        if(_character_builder)
        {
            sp<RenderObject> renderObject = _character_builder->build(scope);
            renderObject->setType(i.character());
            renderObject->setPosition(sp<Vec3::Const>::make(i.position()));
            renderObject->setSize(size);
            renderObjects.push_back(std::move(renderObject));
        }
        else
            renderObjects.push_back(sp<RenderObject>::make(i.character(), sp<Vec3>::make<Vec3::Const>(i.position()), std::move(size)));
    }
    return renderObjects;
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
