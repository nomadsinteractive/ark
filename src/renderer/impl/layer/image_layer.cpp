#include "renderer/impl/layer/image_layer.h"

#include "core/base/bean_factory.h"

#include "renderer/base/atlas.h"

#include "renderer/base/gl_shader.h"
#include "renderer/base/gl_texture.h"
#include "renderer/impl/gl_model/gl_model_quad.h"

namespace ark {

ImageLayer::ImageLayer(const sp<GLShader>& shader, const sp<Atlas>& atlas, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _elements(shader, atlas->texture(), sp<GLModelQuad>::make(shader, atlas), resourceLoaderContext), _atlas(atlas)
{
}

sp<RenderCommand> ImageLayer::render(const LayerContext::Snapshot& renderContext, float x, float y)
{
    return _elements.render(renderContext, x, y);
}

const sp<Atlas>& ImageLayer::atlas() const
{
    return _atlas;
}

ImageLayer::BUILDER::BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _resource_loader_context(resourceLoaderContext), _atlas(factory.ensureBuilder<Atlas>(manifest)),
      _shader(GLShader::fromDocument(factory, manifest, resourceLoaderContext)) {
}

sp<Layer> ImageLayer::BUILDER::build(const sp<Scope>& args)
{
    const sp<Atlas> atlas = _atlas->build(args);
    return sp<ImageLayer>::make(_shader->build(args), atlas, _resource_loader_context);
}

}
