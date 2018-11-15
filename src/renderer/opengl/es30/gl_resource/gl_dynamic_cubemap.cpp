#include "renderer/opengl/es30/gl_resource/gl_dynamic_cubemap.h"

#include "graphics/base/size.h"

#include "renderer/base/gl_resource_manager.h"
#include "renderer/base/gl_shader.h"
#include "renderer/base/resource_loader_context.h"
#include "renderer/util/gl_util.h"

namespace ark {

GLDynamicCubemap::GLDynamicCubemap(const sp<GLResourceManager>& resourceManager, Format format, Feature features, const sp<GLShader>& shader, const sp<GLTexture>& texture, const sp<Size>& size)
    : GLTexture(resourceManager->recycler(), size, static_cast<uint32_t>(GL_TEXTURE_CUBE_MAP), format, features), _resource_manager(resourceManager), _shader(shader), _texture(texture)
{
}

void GLDynamicCubemap::doPrepareTexture(GraphicsContext& graphicsContext, uint32_t id)
{
    GLUtil::renderCubemap(graphicsContext, id, _resource_manager, _shader, _texture, width(), height());
}

GLDynamicCubemap::BUILDER::BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _resource_manager(resourceLoaderContext->glResourceManager()), _size(factory.ensureConcreteClassBuilder<Size>(manifest, Constants::Attributes::SIZE)),
      _shader(GLShader::fromDocument(factory, manifest, resourceLoaderContext, "shaders/equirectangular.vert", "shaders/equirectangular.frag")),
      _texture(factory.ensureBuilder<GLTexture>(manifest, Constants::Attributes::TEXTURE)),
      _format(Documents::getAttribute<GLTexture::Format>(manifest, "format", FORMAT_AUTO)),
      _features(Documents::getAttribute<GLTexture::Feature>(manifest, "feature", FEATURE_DEFAULT))
{
}

sp<GLTexture> GLDynamicCubemap::BUILDER::build(const sp<Scope>& args)
{
    return _resource_manager->createGLResource<GLDynamicCubemap>(_resource_manager, _format, _features, _shader->build(args), _texture->build(args), _size->build(args));
}

}
