#include "graphics/impl/frame/text.h"

#include "core/base/bean_factory.h"
#include "core/util/documents.h"

#include "graphics/base/layer_context.h"
#include "graphics/inf/layer.h"

#include "renderer/base/characters.h"

namespace ark {

Text::Text(const sp<Characters>& characters)
    : _characters(characters)
{
}

void Text::render(RenderRequest& /*pipeline*/, float x, float y)
{
    _characters->renderRequest(V2(x, y));
}

const sp<Size>& Text::size()
{
    return _characters->size();
}

Text::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _layout_param(factory.ensureBuilder<LayoutParam>(manifest)), _text(Strings::load(manifest, Constants::Attributes::TEXT, ""))
{
    const String cid = Documents::getAttribute(manifest, "characters");
    _characters = cid ? factory.ensureBuilder<Characters>(cid) : factory.ensureBuilder<Characters>(manifest);
}

sp<Renderer> Text::BUILDER::build(const sp<Scope>& args)
{
    const sp<Characters> chars = _characters->build(args);
    const sp<LayoutParam> layoutParam = _layout_param->build(args);
    const sp<String> text = _text->build(args);
    if(layoutParam)
        chars->setLayoutParam(layoutParam);
    if(text)
        chars->setText(Strings::fromUTF8(text));
    return sp<Text>::make(chars);
}

}
