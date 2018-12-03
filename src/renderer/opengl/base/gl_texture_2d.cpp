#include "renderer/opengl/base/gl_texture_2d.h"

#include "core/inf/array.h"
#include "core/inf/variable.h"
#include "core/util/conversions.h"
#include "core/util/documents.h"
#include "core/util/log.h"

#include "graphics/base/bitmap.h"
#include "graphics/base/size.h"

#include "renderer/opengl/base/gl_pipeline.h"
#include "renderer/base/gl_recycler.h"
#include "renderer/base/gl_resource_manager.h"
#include "renderer/base/gl_texture_loader.h"
#include "renderer/base/graphics_context.h"
#include "renderer/base/resource_loader_context.h"
#include "renderer/opengl/util/gl_util.h"

namespace ark {

GLTexture2D::GLTexture2D(const sp<GLRecycler>& recycler, const sp<Size>& size, Format format, Feature features, const sp<Variable<sp<Bitmap>>>& bitmap)
    : Texture(recycler, size, static_cast<uint32_t>(GL_TEXTURE_2D), format, features), _bitmap(bitmap)
{
}

void GLTexture2D::doPrepareTexture(GraphicsContext& /*graphicsContext*/, uint32_t id)
{
    const sp<Bitmap> bitmap = _bitmap ? _bitmap->val() : sp<Bitmap>::null();
    uint8_t channels = bitmap ? bitmap->channels() : 4;
    GLenum format = GLUtil::getTextureFormat(_format, channels);
    GLenum pixelFormat = bitmap ? GLUtil::getPixelFormat(_format, bitmap) : GL_UNSIGNED_BYTE;
    GLenum internalFormat = bitmap ? GLUtil::getTextureInternalFormat(_format, bitmap) : GL_RGBA8;
    glTexImage2D(GL_TEXTURE_2D, 0, (GLint) internalFormat, width(), height(), 0, format, pixelFormat, bitmap ? bitmap->at(0, 0) : nullptr);
    LOGD("Uploaded, id = %d, width = %d, height = %d%s", _id, width(), height(), bitmap ? "" : ", bitmap: nullptr");
}

GLTexture2D::DICTIONARY::DICTIONARY(BeanFactory& /*factory*/, const String& value, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _resource_loader_context(resourceLoaderContext), _src(value)
{
}

sp<Texture> GLTexture2D::DICTIONARY::build(const sp<Scope>& /*args*/)
{
    return _resource_loader_context->textureLoader()->get(_src);
}

GLTexture2D::BUILDER::BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _resource_loader_context(resourceLoaderContext), _factory(factory), _manifest(manifest), _src(factory.getBuilder<String>(manifest, Constants::Attributes::SRC)),
      _format(Documents::getAttribute<Texture::Format>(manifest, "format", FORMAT_AUTO)),
      _features(Documents::getAttribute<Texture::Feature>(manifest, "feature", FEATURE_DEFAULT))
{
}

sp<Texture> GLTexture2D::BUILDER::build(const sp<Scope>& args)
{
    const sp<String> src = _src->build(args);
    if(src)
       return _resource_loader_context->textureLoader()->get(*src);

    const sp<Size> size = _factory.ensureConcreteClassBuilder<Size>(_manifest, Constants::Attributes::SIZE)->build(args);
    const sp<Texture> texture = _resource_loader_context->resourceManager()->createGLResource<GLTexture2D>(_resource_loader_context->resourceManager()->recycler(), size, _format, _features, nullptr);
    texture->setTexParameters(_manifest);
    return texture;
}

}
