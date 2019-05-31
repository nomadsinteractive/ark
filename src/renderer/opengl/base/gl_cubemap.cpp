#include "renderer/opengl/base/gl_cubemap.h"

#include "core/inf/variable.h"
#include "core/util/bean_utils.h"
#include "core/util/documents.h"
#include "core/util/log.h"

#include "graphics/base/bitmap.h"
#include "graphics/base/bitmap_bundle.h"
#include "graphics/base/size.h"

#include "renderer/base/recycler.h"
#include "renderer/base/render_controller.h"
#include "renderer/base/resource_loader_context.h"
#include "renderer/base/texture.h"
#include "renderer/opengl/util/gl_util.h"

namespace ark {

namespace {

class TextureUploaderCubemap : public Texture::Uploader {
public:
    TextureUploaderCubemap(std::vector<sp<Variable<bitmap>>> bitmaps)
        : _bitmaps(std::move(bitmaps)) {
    }

    virtual void upload(GraphicsContext& graphicContext, Texture::Delegate& delegate) override {
        for(size_t i = 0; i < _bitmaps.size(); ++i)
            delegate.uploadBitmap(graphicContext, static_cast<uint32_t>(i), _bitmaps.at(i)->val());
    }

private:
    std::vector<sp<Variable<bitmap>>> _bitmaps;
};

}


GLCubemap::GLCubemap(const sp<Recycler>& recycler, const sp<Size>& size, const sp<Texture::Parameters>& parameters, std::vector<sp<Variable<bitmap>>> bitmaps)
    : GLCubemap(recycler, size, parameters, sp<TextureUploaderCubemap>::make(std::move(bitmaps)))
{
}

GLCubemap::GLCubemap(const sp<Recycler>& recycler, const sp<Size>& size, const sp<Texture::Parameters>& parameters, const sp<Texture::Uploader>& uploader)
    : GLTexture(recycler, size, static_cast<uint32_t>(GL_TEXTURE_CUBE_MAP), Texture::TYPE_CUBEMAP, parameters, uploader)
{
}

bool GLCubemap::download(GraphicsContext& /*graphicsContext*/, Bitmap& /*bitmap*/)
{
    return false;
}

void GLCubemap::uploadBitmap(GraphicsContext& /*graphicContext*/, uint32_t index, const Bitmap& bitmap)
{
    uint8_t channels = bitmap.channels();
    GLenum format = GLUtil::getTextureFormat(_parameters->_format, channels);
    GLenum pixelFormat = GLUtil::getPixelFormat(_parameters->_format, bitmap);
    GLenum internalFormat = GLUtil::getTextureInternalFormat(_parameters->_format, bitmap);
    glTexImage2D(static_cast<GLenum>(GL_TEXTURE_CUBE_MAP_POSITIVE_X + index), 0, static_cast<GLint>(internalFormat), static_cast<int32_t>(bitmap.width()), static_cast<int32_t>(bitmap.height()), 0, format, pixelFormat, bitmap.at(0, 0));
    LOGD("GLCubemap Uploaded, id = %d, width = %d, height = %d", static_cast<uint32_t>(id()), bitmap.width(), bitmap.height());
}

}
