#include "graphics/impl/frame/label.h"

#include "core/base/bean_factory.h"

#include "graphics/base/text.h"

namespace ark {

Label::Label(const sp<Text>& characters)
    : _characters(characters)
{
}

void Label::render(RenderRequest& renderequest, const V3& position)
{
//    _characters->render(renderequest, position);
}

const sp<Size>& Label::size()
{
    return _characters->size();
}

Label::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _characters(factory.ensureConcreteClassBuilder<Text>(manifest, "characters")), _text(factory.getBuilder<String>(manifest, Constants::Attributes::TEXT))
{
}

sp<Renderer> Label::BUILDER::build(const Scope& args)
{
    const sp<Text> chars = _characters->build(args);
    const sp<String> text = _text->build(args);
    if(text)
        chars->setText(Strings::fromUTF8(text));
    return sp<Label>::make(chars);
}

}
