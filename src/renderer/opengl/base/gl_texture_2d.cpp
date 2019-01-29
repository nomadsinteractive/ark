#include "renderer/opengl/base/gl_texture_2d.h"

#include "core/inf/array.h"
#include "core/inf/variable.h"
#include "core/util/conversions.h"
#include "core/util/documents.h"
#include "core/util/log.h"

#include "graphics/base/bitmap.h"
#include "graphics/base/size.h"

#include "renderer/opengl/base/gl_pipeline.h"
#include "renderer/base/recycler.h"
#include "renderer/base/render_controller.h"
#include "renderer/base/texture_bundle.h"
#include "renderer/base/graphics_context.h"
#include "renderer/base/resource_loader_context.h"
#include "renderer/base/texture.h"

#include "renderer/opengl/util/gl_util.h"

namespace ark {

GLTexture2D::GLTexture2D(const sp<Recycler>& recycler, const sp<Size>& size, const sp<Texture::Parameters>& parameters, const sp<Variable<bitmap>>& bitmap)
    : GLTexture(recycler, size, static_cast<uint32_t>(GL_TEXTURE_2D), parameters), _bitmap(bitmap)
{
}

void GLTexture2D::doPrepareTexture(GraphicsContext& /*graphicsContext*/, uint32_t id)
{
    const sp<Bitmap> bitmap = _bitmap ? _bitmap->val() : sp<Bitmap>::null();
    uint8_t channels = bitmap ? bitmap->channels() : 4;
    GLenum format = GLUtil::getTextureFormat(_parameters->_format, channels);
    GLenum pixelFormat = bitmap ? GLUtil::getPixelFormat(_parameters->_format, bitmap) : GL_UNSIGNED_BYTE;
    GLenum internalFormat = bitmap ? GLUtil::getTextureInternalFormat(_parameters->_format, bitmap) : GL_RGBA8;
    glTexImage2D(GL_TEXTURE_2D, 0, (GLint) internalFormat, width(), height(), 0, format, pixelFormat, bitmap ? bitmap->at(0, 0) : nullptr);
    LOGD("Uploaded, id = %d, width = %d, height = %d%s", id, width(), height(), bitmap ? "" : ", bitmap: nullptr");
}

}
