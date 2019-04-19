#include "renderer/opengl/es30/gl_resource/gl_dynamic_cubemap.h"

#include "graphics/base/size.h"

#include "renderer/base/render_controller.h"
#include "renderer/base/shader.h"
#include "renderer/base/shader_bindings.h"
#include "renderer/base/texture.h"
#include "renderer/base/resource_loader_context.h"
#include "renderer/base/texture.h"

#include "renderer/opengl/util/gl_util.h"

namespace ark {

GLDynamicCubemap::GLDynamicCubemap(const sp<RenderController>& renderController, const sp<Texture::Parameters>& parameters, const sp<Shader>& shader, const sp<Texture>& texture, const sp<Size>& size)
    : GLTexture(renderController->recycler(), size, static_cast<uint32_t>(GL_TEXTURE_CUBE_MAP), parameters), _render_controller(renderController), _shader(shader),
      _shader_bindings(sp<ShaderBindings>::make(RenderModel::RENDER_MODE_TRIANGLES, renderController, _shader->pipelineLayout())), _texture(texture)
{
}

bool GLDynamicCubemap::download(GraphicsContext& /*graphicsContext*/, Bitmap& bitmap)
{
    return false;
}

void GLDynamicCubemap::doPrepareTexture(GraphicsContext& graphicsContext, uint32_t id)
{
    GLUtil::renderCubemap(graphicsContext, id, _render_controller, _shader->getPipeline(graphicsContext, _shader_bindings), _texture, width(), height());
}

GLDynamicCubemap::BUILDER::BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _render_controller(resourceLoaderContext->renderController()), _size(factory.ensureConcreteClassBuilder<Size>(manifest, Constants::Attributes::SIZE)),
      _shader(Shader::fromDocument(factory, manifest, resourceLoaderContext, "shaders/equirectangular.vert", "shaders/equirectangular.frag")),
      _texture(factory.ensureBuilder<Texture>(manifest, Constants::Attributes::TEXTURE)), _parameters(sp<Texture::Parameters>::make(manifest))
{
}

sp<Texture> GLDynamicCubemap::BUILDER::build(const sp<Scope>& args)
{
    const sp<Size> size = _size->build(args);
    const sp<GLDynamicCubemap> cubemap = sp<GLDynamicCubemap>::make(_render_controller, _parameters, _shader->build(args), _texture->build(args), size);
    return _render_controller->createResource<Texture>(size, sp<Variable<sp<Texture::Delegate>>::Const>::make(cubemap), Texture::TYPE_CUBEMAP);
}

}
