#include "renderer/impl/layer/point_layer.h"

#include "core/base/bean_factory.h"

#include "graphics/base/render_command_pipeline.h"
#include "renderer/base/atlas.h"

#include "renderer/base/gl_shader.h"
#include "renderer/base/resource_loader_context.h"
#include "renderer/impl/gl_model/gl_model_point.h"

namespace ark {

PointLayer::PointLayer(const sp<GLShader>& shader, const sp<Atlas>& atlas, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _elements(shader, atlas->texture(), sp<GLModelPoint>::make(shader, atlas), resourceLoaderContext)
{
}

sp<RenderCommand> PointLayer::render(const LayerContext& renderContext, float x, float y)
{
    return _elements.render(renderContext, x, y);
}

PointLayer::BUILDER::BUILDER(BeanFactory& parent, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _manifest(manifest), _resource_loader_context(resourceLoaderContext), _atlas(parent.ensureBuilder<Atlas>(manifest)),
      _shader(GLShader::fromDocument(parent, manifest, resourceLoaderContext, "shaders/point.vert", "shaders/point.frag")) {
}

sp<Layer> PointLayer::BUILDER::build(const sp<Scope>& args)
{
    const sp<Atlas> atlas = _atlas->build(args);
    return sp<PointLayer>::make(_shader->build(args), atlas, _resource_loader_context);
}

}
