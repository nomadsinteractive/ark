#include "renderer/opengl/base/gl_cubemap.h"

#include "core/inf/variable.h"
#include "core/util/bean_utils.h"
#include "core/util/documents.h"
#include "core/util/log.h"

#include "graphics/base/bitmap.h"
#include "graphics/traits/size.h"

#include "renderer/base/recycler.h"
#include "renderer/base/render_controller.h"
#include "renderer/base/resource_loader_context.h"
#include "renderer/base/texture.h"
#include "renderer/opengl/util/gl_util.h"

namespace ark {
namespace opengl {

namespace {

class TextureUploaderCubemap : public Texture::Uploader {
public:
    TextureUploaderCubemap(std::vector<sp<Variable<bitmap>>> bitmaps)
        : _bitmaps(std::move(bitmaps)) {
    }

    virtual void initialize(GraphicsContext& graphicContext, Texture::Delegate& delegate) override {
        std::vector<bitmap> bitmaps;
        for(const sp<Variable<bitmap>>& i : _bitmaps)
            bitmaps.push_back(i->val());
        DASSERT(bitmaps.size() > 0);

        std::vector<sp<ByteArray>> imagedata;
        for(const bitmap& i : bitmaps)
            imagedata.push_back(i->byteArray());

        delegate.uploadBitmap(graphicContext, bitmaps.at(0), imagedata);
    }

private:
    std::vector<sp<Variable<bitmap>>> _bitmaps;
};

}


GLCubemap::GLCubemap(sp<Recycler> recycler, sp<Size> size, sp<Texture::Parameters> parameters)
    : GLTexture(std::move(recycler), std::move(size), static_cast<uint32_t>(GL_TEXTURE_CUBE_MAP), Texture::TYPE_CUBEMAP, std::move(parameters))
{
}

void GLCubemap::clear(GraphicsContext& /*graphicsContext*/)
{
    DFATAL("Unimplemented");
}

bool GLCubemap::download(GraphicsContext& /*graphicsContext*/, Bitmap& /*bitmap*/)
{
    return false;
}

void GLCubemap::uploadBitmap(GraphicsContext& /*graphicContext*/, const Bitmap& bitmap, const std::vector<sp<ByteArray>>& imagedata)
{
    uint8_t channels = bitmap.channels();
    GLenum format = GLUtil::getTextureFormat(_parameters->_usage, _parameters->_format, channels);
    GLenum pixelFormat = GLUtil::getPixelType(_parameters->_format, bitmap);
    GLenum internalFormat = GLUtil::getTextureInternalFormat(_parameters->_usage, _parameters->_format, bitmap);
    DASSERT(imagedata.size() <= 6);
    for(size_t i = 0; i < imagedata.size(); ++i)
        glTexImage2D(static_cast<GLenum>(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i), 0, static_cast<GLint>(internalFormat), static_cast<int32_t>(bitmap.width()), static_cast<int32_t>(bitmap.height()), 0, format, pixelFormat, imagedata.at(i)->buf());
    LOGD("GLCubemap Uploaded, id = %d, width = %d, height = %d", static_cast<uint32_t>(id()), bitmap.width(), bitmap.height());
}

}
}
