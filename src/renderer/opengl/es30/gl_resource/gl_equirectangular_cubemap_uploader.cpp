#include "renderer/opengl/es30/gl_resource/gl_equirectangular_cubemap_uploader.h"

#include "graphics/traits/size.h"

#include "renderer/base/shader.h"
#include "renderer/base/pipeline_bindings.h"
#include "renderer/base/resource_loader_context.h"

#include "renderer/opengl/util/gl_util.h"

namespace ark {

GLEquirectangularCubemapUploader::GLEquirectangularCubemapUploader(const sp<RenderController>& renderController, const Shader& shader, const sp<Texture>& texture, const sp<Size>& size)
    : _render_controller(renderController), _pipeline_bindings(shader.makeBindings(Buffer(), Enum::RENDER_MODE_TRIANGLES, Enum::DRAW_PROCEDURE_DRAW_ELEMENTS)), _texture(texture), _size(size)
{
}

void GLEquirectangularCubemapUploader::initialize(GraphicsContext& graphicsContext, Texture::Delegate& delegate)
{
    DCHECK(delegate.type() == Texture::TYPE_CUBEMAP, "This uploader uploads bitmaps to a cubmap, not Texture::Type(%d)", delegate.type());
    GLUtil::renderCubemap(graphicsContext, static_cast<uint32_t>(delegate.id()), _render_controller, _pipeline_bindings->ensureRenderPipeline(graphicsContext), _texture,
                          static_cast<int32_t>(_size->widthAsFloat()), static_cast<int32_t>(_size->heightAsFloat()));
}

GLEquirectangularCubemapUploader::BUILDER::BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _render_controller(resourceLoaderContext->renderController()), _size(factory.ensureConcreteClassBuilder<Size>(manifest, constants::SIZE)),
      _shader(Shader::fromDocument(factory, manifest, resourceLoaderContext, "shaders/equirectangular.vert", "shaders/equirectangular.frag")),
      _texture(factory.ensureBuilder<Texture>(manifest, constants::TEXTURE))
{
}

sp<Texture::Uploader> GLEquirectangularCubemapUploader::BUILDER::build(const Scope& args)
{
    return sp<GLEquirectangularCubemapUploader>::make(_render_controller, _shader->build(args), _texture->build(args), _size->build(args));
}

}
