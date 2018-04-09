#include "renderer/base/gl_texture.h"

#include "core/inf/array.h"
#include "core/inf/variable.h"
#include "core/util/documents.h"
#include "core/util/log.h"

#include "graphics/base/bitmap.h"
#include "graphics/base/size.h"

#include "renderer/base/gl_program.h"
#include "renderer/base/gl_recycler.h"
#include "renderer/base/gl_resource_manager.h"
#include "renderer/base/gl_texture_loader.h"
#include "renderer/base/graphics_context.h"
#include "renderer/base/resource_loader_context.h"
#include "renderer/util/gl_contants.h"

namespace ark {

static GLenum getTextureInternalFormat(GLTexture::Format format, const Bitmap& bitmap) {
    const GLenum formats[] = {GL_R8, GL_R8_SNORM, GL_R16, GL_R16_SNORM,
                              GL_RG8, GL_RG8_SNORM, GL_RG16, GL_RG16_SNORM,
                              GL_RGB8, GL_RGB8_SNORM, GL_RGB16, GL_RGB16_SNORM,
                              GL_RGBA8, GL_RGBA8_SNORM, GL_RGBA16, GL_RGBA16_SNORM};
    uint32_t signedOffset = (format & GLTexture::FORMAT_SIGNED) == GLTexture::FORMAT_SIGNED ? 1 : 0;
    uint32_t byteCount = bitmap.rowBytes() / bitmap.width() / bitmap.channels();
    uint32_t channel4 = (bitmap.channels() - 1) * 4;
    DCHECK(byteCount <= 2, "Unsupported color depth: %d", byteCount * 8);
    return format == GLTexture::FORMAT_AUTO ? formats[channel4] : formats[channel4 + (byteCount - 1) * 2 + signedOffset];
}

static GLenum getTextureFormat(GLTexture::Format format, uint8_t channels) {
    const GLenum formatByChannels[] = {GL_RED, GL_RG, GL_RGB, GL_RGBA};
    DCHECK(channels < 5, "Unknown bitmap format: (channels = %d)", static_cast<uint32_t>(channels));
    return format == GLTexture::FORMAT_AUTO ? formatByChannels[channels - 1] : formatByChannels[static_cast<uint32_t>(format & GLTexture::FORMAT_RGBA)];
}

static GLenum getPixelFormat(GLTexture::Format format, const Bitmap& bitmap) {
    bool flagSigned = (format & GLTexture::FORMAT_SIGNED) == GLTexture::FORMAT_SIGNED;
    uint32_t byteCount = bitmap.rowBytes() / bitmap.width() / bitmap.channels();
    if(byteCount == 1)
        return flagSigned ? GL_BYTE : GL_UNSIGNED_BYTE;
    if(byteCount == 2)
        return flagSigned ? GL_SHORT: GL_UNSIGNED_SHORT;
    return flagSigned ? GL_INT : GL_UNSIGNED_INT;
}

GLTexture::GLTexture(const sp<GLRecycler>& recycler, uint32_t width, uint32_t height, const sp<Variable<sp<Bitmap>>>& bitmap, Format format)
    : _recycler(recycler), _id(0), _width(width), _height(height), _bitmap(bitmap), _format(format)
{
    setTexParameter(static_cast<uint32_t>(GL_TEXTURE_MIN_FILTER), static_cast<int32_t>(GL_LINEAR));
    setTexParameter(static_cast<uint32_t>(GL_TEXTURE_MAG_FILTER), static_cast<int32_t>(GL_LINEAR));
    setTexParameter(static_cast<uint32_t>(GL_TEXTURE_WRAP_S), static_cast<int32_t>(GL_CLAMP_TO_EDGE));
    setTexParameter(static_cast<uint32_t>(GL_TEXTURE_WRAP_T), static_cast<int32_t>(GL_CLAMP_TO_EDGE));
}

GLTexture::~GLTexture()
{
    if(_id > 0)
        _recycler->recycle(sp<Recycler>::make(_id));
}

void GLTexture::setTexParameters(const document& doc)
{
    for(const document& i : doc->children("parameter"))
        _tex_parameters[static_cast<uint32_t>(_gl_constants->getEnum(Documents::ensureAttribute(i, Constants::Attributes::NAME)))] = static_cast<int32_t>(_gl_constants->getEnum(Documents::ensureAttribute(i, Constants::Attributes::VALUE)));
}

void GLTexture::setTexParameter(uint32_t name, int32_t value)
{
    _tex_parameters[name] = value;
}

void GLTexture::setTexFormat(GLTexture::Format format)
{
    _format = format;
}

void GLTexture::prepare(GraphicsContext& /*graphicsContext*/)
{
    const sp<Bitmap> bitmap = _bitmap ? _bitmap->val() : sp<Bitmap>::null();
    if(_id == 0)
        glGenTextures(1, &_id);
    glBindTexture(GL_TEXTURE_2D, _id);
    uint8_t channels = bitmap ? bitmap->channels() : 4;
    GLenum format = getTextureFormat(_format, channels);
    GLenum pixelFormat = bitmap ? getPixelFormat(_format, bitmap) : GL_UNSIGNED_BYTE;
    GLenum internalFormat = bitmap ? getTextureInternalFormat(_format, bitmap) : GL_RGBA8;
    glTexImage2D(GL_TEXTURE_2D, 0, (GLint) internalFormat, static_cast<int32_t>(_width), static_cast<int32_t>(_height), 0, format, pixelFormat, bitmap ? bitmap->at(0, 0) : nullptr);
    for(const auto i : _tex_parameters)
        glTexParameteri(GL_TEXTURE_2D, static_cast<GLenum>(i.first), static_cast<GLint>(i.second));
    LOGD("Uploaded, id = %d, width = %d, height = %d%s", _id, _width, _height, bitmap ? "" : ", bitmap: nullptr");
}

void GLTexture::recycle(GraphicsContext& /*graphicsContext*/)
{
    LOGD("Deleting GLTexture[%d]", _id);
    glDeleteTextures(1, &_id);
    _id = 0;
}

uint32_t GLTexture::id()
{
    return _id;
}

uint32_t GLTexture::width() const
{
    return _width;
}

uint32_t GLTexture::height() const
{
    return _height;
}

sp<Bitmap> GLTexture::getBitmap() const
{
    return _bitmap->val();
}

void GLTexture::active(const sp<GLProgram>& program, uint32_t id) const
{
    glActiveTexture(static_cast<GLenum>(GL_TEXTURE0 + id));
    glBindTexture(GL_TEXTURE_2D, _id);

    char uniformName[16] = "u_Texture0";
    uniformName[9] = static_cast<char>('0' + id);
    const GLProgram::Uniform& uTexture = program->getUniform(uniformName);
    DCHECK(uTexture, "Texture %d not declared in shader", id);
    uTexture.setUniform1i(id);
}

GLTexture::Recycler::Recycler(uint32_t id)
    : _id(id)
{
}

uint32_t GLTexture::Recycler::id()
{
    return _id;
}

void GLTexture::Recycler::prepare(GraphicsContext&)
{
}

void GLTexture::Recycler::recycle(GraphicsContext&)
{
    LOGD("Deleting GLTexture[%d]", _id);
    glDeleteTextures(1, &_id);
    _id = 0;
}

GLTexture::DICTIONARY::DICTIONARY(BeanFactory& /*parent*/, const String& value, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _resource_loader_context(resourceLoaderContext), _src(value)
{
}

sp<GLTexture> GLTexture::DICTIONARY::build(const sp<Scope>& /*args*/)
{
    return _resource_loader_context->textureLoader()->get(_src);
}

GLTexture::BUILDER::BUILDER(BeanFactory& factory, const document& doc, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _resource_loader_context(resourceLoaderContext), _factory(factory), _manifest(doc), _src(Strings::load(doc, Constants::Attributes::SRC, ""))
{
    const String v = Documents::getAttribute(doc, "format");
    if(v)
    {
        _format = FORMAT_ALPHA;
        for(const String& format : v.toLower().split('|'))
        {
            if(format == "alpha")
                _format = static_cast<Format>(_format | GLTexture::FORMAT_ALPHA);
            else if(format == "rg")
                _format = static_cast<Format>(_format | GLTexture::FORMAT_RG);
            else if(format == "rgb")
                _format = static_cast<Format>(_format | GLTexture::FORMAT_RGB);
            else if(format == "rgba")
                _format = static_cast<Format>(_format | GLTexture::FORMAT_RGBA);

            if(format == "signed")
                _format = static_cast<Format>(_format | GLTexture::FORMAT_SIGNED);
        }
    }
    else
        _format = GLTexture::FORMAT_AUTO;
}

sp<GLTexture> GLTexture::BUILDER::build(const sp<Scope>& args)
{
    const String src = _src->build(args);
    if(src)
    {
        const sp<GLTexture> texture = _resource_loader_context->textureLoader()->get(src);
        texture->setTexParameters(_manifest);
        texture->setTexFormat(_format);
        return texture;
    }
    const sp<Size> size = _factory.ensure<Size>(_manifest, args);
    return _resource_loader_context->glResourceManager()->createGLTexture(static_cast<uint32_t>(size->width()), static_cast<uint32_t>(size->height()), nullptr);
}

}
