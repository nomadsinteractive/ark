#include "impl/texture/gl_radiance_cubemap.h"

#include <cmft/cubemapfilter.h>

#include "core/util/math.h"
#include "core/util/log.h"

#include "graphics/base/bitmap.h"
#include "graphics/base/size.h"

#include "renderer/base/gl_resource_manager.h"
#include "renderer/base/gl_shader.h"
#include "renderer/base/gl_texture_default.h"
#include "renderer/base/resource_loader_context.h"
#include "renderer/util/gl_util.h"

#include "platform/gl/gl.h"

namespace ark {

GLRadianceCubemap::GLRadianceCubemap(const sp<GLResourceManager>& resourceManager, Format format, Feature features, const sp<GLTexture>& texture, const sp<Size>& size)
    : GLTexture(resourceManager->recycler(), size, static_cast<uint32_t>(GL_TEXTURE_CUBE_MAP), format, features), _resource_manager(resourceManager), _texture(texture)
{
}

void GLRadianceCubemap::doPrepareTexture(GraphicsContext& graphicsContext, uint32_t id)
{
    DCHECK(_size->width() == _size->height(), "Cubemap should be square, but (%.2f, %.2f) provided", _size->width(), _size->height());

    cmft::Image input;
    cmft::imageCreate(input, _texture->width(), _texture->height(), 0, 1, 1, cmft::TextureFormat::RGBA32F);

    if(!_texture->id())
        _texture->prepare(graphicsContext);

    glBindTexture(GL_TEXTURE_2D, _texture->id());
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, input.m_data);
    glBindTexture(GL_TEXTURE_2D, 0);

    uint32_t n = static_cast<uint32_t>(_size->width());

    cmft::Image output;
    cmft::imageCreate(output, n, n, 0, 1, 6, cmft::TextureFormat::RGBA32F);
    cmft::imageToCubemap(input);
//    cmft::imageIrradianceFilterSh(output, n, input);
    cmft::imageRadianceFilter(output, n, cmft::LightingModel::BlinnBrdf, true, 1, 8, 1, input, cmft::EdgeFixup::None, 8);

    cmft::Image faceList[6];
    cmft::imageFaceListFromCubemap(faceList, output);

    for(size_t i = 0; i < 6; ++i)
    {
        GLenum format = GL_RGBA;
        GLenum pixelFormat = GL_FLOAT;
        GLenum internalFormat = GL_RGBA16F;
        glTexImage2D(static_cast<GLenum>(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i), 0, (GLint) internalFormat, static_cast<int32_t>(n), static_cast<int32_t>(n), 0, format, pixelFormat, faceList[i].m_data);
        LOGD("GLCubemap Uploaded, id = %d, width = %d, height = %d", id, n, n);
    }

    cmft::imageUnload(input);
    cmft::imageUnload(output);
}

GLRadianceCubemap::BUILDER::BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _resource_manager(resourceLoaderContext->glResourceManager()), _size(factory.ensureConcreteClassBuilder<Size>(manifest, Constants::Attributes::SIZE)),
      _texture(factory.ensureBuilder<GLTexture>(manifest, Constants::Attributes::TEXTURE)),
      _format(Documents::getAttribute<GLTexture::Format>(manifest, "format", FORMAT_AUTO)),
      _features(Documents::getAttribute<GLTexture::Feature>(manifest, "feature", FEATURE_DEFAULT))
{
}

sp<GLTexture> GLRadianceCubemap::BUILDER::build(const sp<Scope>& args)
{
    return _resource_manager->createGLResource<GLRadianceCubemap>(_resource_manager, _format, _features, _texture->build(args), _size->build(args));
}

}
