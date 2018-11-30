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
#include "renderer/opengl/util/gl_util.h"

namespace ark {

GLCubemap::GLCubemap(const sp<GLRecycler>& recycler, const sp<Size>& size, Texture::Format format, Texture::Feature features, std::vector<sp<Variable<bitmap>>> bitmaps)
    : Texture(recycler, size, static_cast<uint32_t>(GL_TEXTURE_CUBE_MAP), format, features), _bitmaps(std::move(bitmaps))
{
}

void GLCubemap::doPrepareTexture(GraphicsContext& /*graphicsContext*/, uint32_t id)
{
    for(size_t i = 0; i < _bitmaps.size(); ++i)
    {
        const sp<Bitmap> bitmap = _bitmaps[i] ? _bitmaps[i]->val() : sp<Bitmap>::null();
        uint8_t channels = bitmap ? bitmap->channels() : 4;
        GLenum format = GLUtil::getTextureFormat(_format, channels);
        GLenum pixelFormat = bitmap ? GLUtil::getPixelFormat(_format, bitmap) : GL_UNSIGNED_BYTE;
        GLenum internalFormat = bitmap ? GLUtil::getTextureInternalFormat(_format, bitmap) : GL_RGBA8;
        glTexImage2D(static_cast<GLenum>(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i), 0, (GLint) internalFormat, static_cast<int32_t>(bitmap->width()), static_cast<int32_t>(bitmap->height()), 0, format, pixelFormat, bitmap ? bitmap->at(0, 0) : nullptr);
        LOGD("GLCubemap Uploaded, id = %d, width = %d, height = %d%s", id, bitmap->width(), bitmap->height(), bitmap ? "" : ", bitmap: nullptr");
    }
}

GLCubemap::BUILDER::BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _resource_loader_context(resourceLoaderContext), _factory(factory), _manifest(manifest), _size(factory.ensureConcreteClassBuilder<Size>(manifest, Constants::Attributes::SIZE)),
      _format(Documents::getAttribute<Texture::Format>(manifest, "format", Texture::FORMAT_AUTO)),
      _features(Documents::getAttribute<Texture::Feature>(manifest, "feature", Texture::FEATURE_DEFAULT))
{
    BeanUtils::split(factory, manifest, Constants::Attributes::SRC, _srcs[0], _srcs[1], _srcs[2], _srcs[3], _srcs[4], _srcs[5]);
}

sp<GLCubemap> GLCubemap::BUILDER::build(const sp<Scope>& args)
{
    std::vector<sp<Variable<bitmap>>> bitmaps;
    for(size_t i = 0; i < 6; ++i)
    {
        const String src = _srcs[i]->build(args);
        bitmaps.push_back(sp<typename Variable<bitmap>::Get>::make(_resource_loader_context->images(), src));
    }
    return _resource_loader_context->resourceManager()->createGLResource<GLCubemap>(_resource_loader_context->resourceManager()->recycler(), _size->build(args), _format, _features, std::move(bitmaps));
}

GLCubemap::BUILDER_IMPL1::BUILDER_IMPL1(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _delegate(factory, manifest, resourceLoaderContext)
{
}

sp<Texture> GLCubemap::BUILDER_IMPL1::build(const sp<Scope>& args)
{
    return _delegate.build(args);
}

}
