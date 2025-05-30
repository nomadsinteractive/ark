#include "opengl/impl/es30/gl_resource/gl_equirectangular_cubemap_uploader.h"

#include "graphics/components/size.h"

#include "renderer/base/shader.h"
#include "renderer/base/pipeline_bindings.h"
#include "renderer/base/resource_loader_context.h"

#include "opengl/util/gl_util.h"

namespace ark::plugin::opengl {

GLEquirectangularCubemapUploader::GLEquirectangularCubemapUploader(const Shader& shader, const sp<Texture>& texture, const sp<Size>& size)
    : _pipeline_bindings(shader.makeBindings(Buffer(), enums::DRAW_MODE_TRIANGLES, enums::DRAW_PROCEDURE_DRAW_ELEMENTS)), _texture(texture), _size(size)
{
}

void GLEquirectangularCubemapUploader::initialize(GraphicsContext& graphicsContext, Texture::Delegate& delegate)
{
    DCHECK(delegate.type() == Texture::TYPE_CUBEMAP, "This uploader uploads bitmaps to a cubmap, not Texture::Type(%d)", delegate.type());
    GLUtil::renderCubemap(graphicsContext, static_cast<uint32_t>(delegate.id()), Ark::instance().renderController(), _pipeline_bindings->ensureRenderPipeline(graphicsContext), _texture,
                          static_cast<int32_t>(_size->widthAsFloat()), static_cast<int32_t>(_size->heightAsFloat()));
}

GLEquirectangularCubemapUploader::BUILDER::BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _size(factory.ensureConcreteClassBuilder<Size>(manifest, constants::SIZE)), _shader(Shader::fromDocument(factory, manifest, "shaders/equirectangular.vert", "shaders/equirectangular.frag")),
      _texture(factory.ensureBuilder<Texture>(manifest, constants::TEXTURE))
{
}

sp<Texture::Uploader> GLEquirectangularCubemapUploader::BUILDER::build(const Scope& args)
{
    return sp<Texture::Uploader>::make<GLEquirectangularCubemapUploader>(_shader->build(args), _texture->build(args), _size->build(args));
}

}
