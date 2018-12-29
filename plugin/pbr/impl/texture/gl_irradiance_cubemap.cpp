#include "impl/texture/gl_irradiance_cubemap.h"

#include <cmft/cubemapfilter.h>

#include "core/util/log.h"

#include "graphics/base/bitmap.h"
#include "graphics/base/size.h"

#include "renderer/base/resource_manager.h"
#include "renderer/base/shader.h"
#include "renderer/opengl/base/gl_texture_2d.h"
#include "renderer/base/resource_loader_context.h"
#include "renderer/opengl/util/gl_util.h"

#include "platform/gl/gl.h"

namespace ark {

GLIrradianceCubemap::GLIrradianceCubemap(const sp<ResourceManager>& resourceManager, const sp<Texture::Parameters>& params, const sp<Shader>& shader, const sp<Texture>& texture, const sp<Size>& size)
    : GLTexture(resourceManager->recycler(), size, static_cast<uint32_t>(GL_TEXTURE_CUBE_MAP), params), _resource_manager(resourceManager), _shader(shader), _texture(texture)
{
}

void GLIrradianceCubemap::doPrepareTexture(GraphicsContext& graphicsContext, uint32_t id)
{
    DCHECK(_size->width() == _size->height(), "Cubemap should be square, but (%.2f, %.2f) provided", _size->width(), _size->height());

    cmft::Image input;
    cmft::imageCreate(input, _texture->width(), _texture->height(), 0, 1, 1, cmft::TextureFormat::RGBA32F);

    if(!_texture->id())
        _texture->upload(graphicsContext);

    glBindTexture(GL_TEXTURE_2D, _texture->id());
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, input.m_data);
    glBindTexture(GL_TEXTURE_2D, 0);

    uint32_t n = static_cast<uint32_t>(_size->width());

    cmft::Image output;
    cmft::imageCreate(output, n, n, 0, 1, 6, cmft::TextureFormat::RGBA32F);
    cmft::imageToCubemap(input);
    cmft::imageIrradianceFilterSh(output, n, input);

    cmft::Image faceList[6];
    cmft::imageFaceListFromCubemap(faceList, output);

    const uint32_t imageFaceIndices[6] = {4, 5, 2, 3, 1, 0};

    Bitmap::Util::rotate<float>(reinterpret_cast<float*>(faceList[2].m_data), n, n, 4, 270);
    Bitmap::Util::hvflip<float>(reinterpret_cast<float*>(faceList[3].m_data), n, n, 4);
    Bitmap::Util::hflip<float>(reinterpret_cast<float*>(faceList[3].m_data), n, n, 4);

    for(uint32_t i = 0; i < 6; ++i)
    {
        GLenum format = GL_RGBA;
        GLenum pixelFormat = GL_FLOAT;
        GLenum internalFormat = GL_RGB16F;
        glTexImage2D(static_cast<GLenum>(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i), 0, (GLint) internalFormat, static_cast<int32_t>(n), static_cast<int32_t>(n), 0, format, pixelFormat, faceList[imageFaceIndices[i]].m_data);
        LOGD("GLCubemap Uploaded, id = %d, width = %d, height = %d", id, n, n);
    }

    cmft::imageUnload(input);
    cmft::imageUnload(output);
}

GLIrradianceCubemap::BUILDER::BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _resource_manager(resourceLoaderContext->resourceManager()), _size(factory.ensureConcreteClassBuilder<Size>(manifest, Constants::Attributes::SIZE)),
      _shader(Shader::fromDocument(factory, manifest, resourceLoaderContext, "shaders/equirectangular.vert", "shaders/equirectangular.frag")),
      _texture(factory.ensureBuilder<Texture>(manifest, Constants::Attributes::TEXTURE)), _parameters(sp<Texture::Parameters>::make(manifest))
{
}

sp<Texture> GLIrradianceCubemap::BUILDER::build(const sp<Scope>& args)
{
    const sp<Size> size = _size->build(args);
    const sp<GLIrradianceCubemap> cubemap = sp<GLIrradianceCubemap>::make(_resource_manager, _parameters, _shader->build(args), _texture->build(args), _size->build(args));
    return _resource_manager->createGLResource<Texture>(size, sp<Variable<sp<Resource>>::Const>::make(cubemap), Texture::TYPE_CUBEMAP);
}

}
