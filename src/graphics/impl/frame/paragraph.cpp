#include "graphics/impl/frame/paragraph.h"

#include "core/base/bean_factory.h"
#include "core/util/documents.h"

#include "graphics/base/layer_context.h"
#include "graphics/base/render_layer.h"
#include "graphics/base/size.h"

#include "graphics/base/text.h"

namespace ark {

Paragraph::Paragraph(const sp<Text>& characters)
    : _characters(characters)
{
}

void Paragraph::render(RenderRequest& renderequest, const V3& position)
{
//    _characters->render(renderequest, position);
}

const sp<Size>& Paragraph::size()
{
    return _characters->size();
}

Paragraph::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _characters(factory.ensureConcreteClassBuilder<Text>(manifest, "characters")), _string(factory.getBuilder<String>(manifest, Constants::Attributes::TEXT))
{
}

sp<Renderer> Paragraph::BUILDER::build(const Scope& args)
{
    const sp<Text> chars = _characters->build(args);
    const sp<String> string = _string->build(args);
    if(string)
        chars->setText(Strings::fromUTF8(string));
    return sp<Paragraph>::make(chars);
}

}
