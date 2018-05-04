#include "graphics/impl/frame/label.h"

#include "core/base/bean_factory.h"
#include "core/util/documents.h"

#include "graphics/base/layer_context.h"
#include "graphics/inf/layer.h"

#include "renderer/base/characters.h"

namespace ark {

Label::Label(const sp<Characters>& characters)
    : _characters(characters)
{
}

void Label::render(RenderRequest& /*pipeline*/, float x, float y)
{
    const sp<LayerContext>& renderContext = _characters->layer()->layerContext();
    for(const sp<RenderObject>& i : _characters->characters())
        renderContext->draw(x, y, i);
}

const sp<Size>& Label::size()
{
    return _characters->size();
}

Label::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _text(Strings::load(manifest, Constants::Attributes::TEXT, ""))
{
    const String cid = Documents::getAttribute(manifest, "characters");
    _characters = cid ? factory.ensureBuilder<Characters>(cid) : factory.ensureBuilder<Characters>(manifest);
}

sp<Renderer> Label::BUILDER::build(const sp<Scope>& args)
{
    const sp<Characters> chars = _characters->build(args);
    const sp<String> text = _text->build(args);
    if(text)
        chars->setText(Strings::fromUTF8(text));
    return sp<Label>::make(chars);
}

}
