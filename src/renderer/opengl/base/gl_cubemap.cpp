#include "renderer/opengl/base/gl_cubemap.h"

#include "core/inf/variable.h"
#include "core/util/bean_utils.h"
#include "core/util/documents.h"
#include "core/util/log.h"

#include "graphics/base/bitmap.h"
#include "graphics/base/image_resource.h"
#include "graphics/base/size.h"

#include "renderer/opengl/base/gl_pipeline.h"
#include "renderer/base/gl_recycler.h"
#include "renderer/base/gl_resource_manager.h"
#include "renderer/base/resource_loader_context.h"
#include "renderer/base/texture.h"
#include "renderer/opengl/util/gl_util.h"

namespace ark {

GLCubemap::GLCubemap(const sp<GLRecycler>& recycler, const sp<Size>& size, const sp<Texture::Parameters>& parameters, std::vector<sp<Variable<bitmap>>> bitmaps)
    : GLTexture(recycler, size, static_cast<uint32_t>(GL_TEXTURE_CUBE_MAP), parameters), _bitmaps(std::move(bitmaps))
{
}

void GLCubemap::doPrepareTexture(GraphicsContext& /*graphicsContext*/, uint32_t id)
{
    for(size_t i = 0; i < _bitmaps.size(); ++i)
    {
        const sp<Bitmap> bitmap = _bitmaps[i] ? _bitmaps[i]->val() : sp<Bitmap>::null();
        uint8_t channels = bitmap ? bitmap->channels() : 4;
        GLenum format = GLUtil::getTextureFormat(_parameters->_format, channels);
        GLenum pixelFormat = bitmap ? GLUtil::getPixelFormat(_parameters->_format, bitmap) : GL_UNSIGNED_BYTE;
        GLenum internalFormat = bitmap ? GLUtil::getTextureInternalFormat(_parameters->_format, bitmap) : GL_RGBA8;
        glTexImage2D(static_cast<GLenum>(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i), 0, (GLint) internalFormat, static_cast<int32_t>(bitmap->width()), static_cast<int32_t>(bitmap->height()), 0, format, pixelFormat, bitmap ? bitmap->at(0, 0) : nullptr);
        LOGD("GLCubemap Uploaded, id = %d, width = %d, height = %d%s", id, bitmap->width(), bitmap->height(), bitmap ? "" : ", bitmap: nullptr");
    }
}

GLCubemap::BUILDER::BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _resource_loader_context(resourceLoaderContext), _factory(factory), _manifest(manifest), _size(factory.ensureConcreteClassBuilder<Size>(manifest, Constants::Attributes::SIZE)),
      _parameters(sp<Texture::Parameters>::make(manifest))
{
    BeanUtils::split(factory, manifest, Constants::Attributes::SRC, _srcs[0], _srcs[1], _srcs[2], _srcs[3], _srcs[4], _srcs[5]);
}

sp<Texture> GLCubemap::BUILDER::build(const sp<Scope>& args)
{
    std::vector<sp<Variable<bitmap>>> bitmaps;
    for(size_t i = 0; i < 6; ++i)
    {
        const String src = _srcs[i]->build(args);
        bitmaps.push_back(sp<typename Variable<bitmap>::Get>::make(_resource_loader_context->images(), src));
    }
    const sp<Size> size = _size->build(args);
    const sp<GLCubemap> cubemap = sp<GLCubemap>::make(_resource_loader_context->resourceManager()->recycler(), size, _parameters, std::move(bitmaps));
    return _resource_loader_context->resourceManager()->createGLResource<Texture>(size, cubemap);
}

}