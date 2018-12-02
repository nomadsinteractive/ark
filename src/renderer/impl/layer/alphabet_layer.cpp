#include "renderer/impl/layer/alphabet_layer.h"

#include "graphics/base/layer.h"

#include "renderer/base/shader.h"
#include "renderer/base/resource_loader_context.h"
#include "renderer/impl/gl_model/gl_model_text.h"
#include "renderer/impl/gl_snippet/gl_snippet_ucolor.h"

namespace ark {

AlphabetLayer::BUILDER::BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _resource_loader_context(resourceLoaderContext),
      _alphabet(factory.ensureBuilder<Alphabet>(manifest, Constants::Attributes::ALPHABET)),
      _shader(factory.getBuilder<Shader>(manifest, Constants::Attributes::SHADER)),
      _color(factory.getBuilder<Vec4>(manifest, Constants::Attributes::TEXT_COLOR)),
      _texture_width(Documents::getAttribute<uint32_t>(manifest, "texture-width", 256)),
      _texture_height(Documents::getAttribute<uint32_t>(manifest, "texture-height", 256))
{
}

sp<Layer> AlphabetLayer::BUILDER::build(const sp<Scope>& args)
{
    const sp<Vec4> color = _color ? _color->build(args) : sp<Color>::make(Color::WHITE).cast<Vec4>();
    const sp<Shader> shader = _shader ? _shader->build(args) : Shader::fromStringTable("shaders/default.vert", "shaders/alpha.frag", sp<GLSnippetUColor>::make(color), _resource_loader_context);
    return sp<Layer>::make(sp<GLModelText>::make(_resource_loader_context->resourceManager(), _alphabet->build(args), 256, 256), shader, _resource_loader_context);
}

}
