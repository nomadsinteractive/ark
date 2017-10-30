#include "graphics/impl/frame/label.h"

#include "core/base/bean_factory.h"
#include "core/util/bean_utils.h"
#include "core/util/documents.h"

#include "graphics/base/size.h"
#include "graphics/base/render_object.h"
#include "graphics/base/layer_context.h"
#include "graphics/inf/alphabet.h"

#include "renderer/base/characters.h"
#include "renderer/impl/layer/image_layer.h"
#include "renderer/impl/layer/alphabet_layer.h"

namespace ark {

Label::Label(const sp<Characters>& characters)
    : _characters(characters)
{
}

void Label::render(RenderCommandPipeline& /*pipeline*/, float x, float y)
{
    const sp<LayerContext>& renderContext = _characters->layer()->renderContext();
    for(const sp<RenderObject>& i : _characters->characters())
        renderContext->draw(x, y, i);
}

const sp<Size>& Label::size()
{
    return _characters->size();
}

Label::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
{
    const String& cid = Documents::getAttribute(manifest, "characters");
    _characters = cid ? factory.ensureBuilder<Characters>(cid) : factory.ensureBuilder<Characters>(manifest);
}

sp<Renderer> Label::BUILDER::build(const sp<Scope>& args)
{
    return sp<Label>::make(_characters->build(args));
}

}
