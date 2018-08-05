#include "renderer/base/gl_texture.h"

#include "core/inf/array.h"
#include "core/inf/variable.h"
#include "core/util/conversions.h"
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
    GLenum format = GLConstants::getTextureFormat(_format, channels);
    GLenum pixelFormat = bitmap ? GLConstants::getPixelFormat(_format, bitmap) : GL_UNSIGNED_BYTE;
    GLenum internalFormat = bitmap ? GLConstants::getTextureInternalFormat(_format, bitmap) : GL_RGBA8;
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
    : _resource_loader_context(resourceLoaderContext), _factory(factory), _manifest(doc), _src(Strings::load(doc, Constants::Attributes::SRC, "")),
      _format(Documents::getAttribute<GLTexture::Format>(doc, "format", FORMAT_AUTO))
{
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


template<> ARK_API GLTexture::Format Conversions::to<String, GLTexture::Format>(const String& str)
{
    if(str)
    {
        GLTexture::Format format = GLTexture::FORMAT_R;
        for(const String& i : str.toLower().split('|'))
        {
            if(i == "r")
                format = static_cast<GLTexture::Format>(format | GLTexture::FORMAT_R);
            else if(i == "rg")
                format = static_cast<GLTexture::Format>(format | GLTexture::FORMAT_RG);
            else if(i == "rgb")
                format = static_cast<GLTexture::Format>(format | GLTexture::FORMAT_RGB);
            else if(i == "rgba")
                format = static_cast<GLTexture::Format>(format | GLTexture::FORMAT_RGBA);
            else if(i == "signed")
                format = static_cast<GLTexture::Format>(format | GLTexture::FORMAT_SIGNED);
            DFATAL("Unknow texture format: %s", i.c_str());
        }
        return format;
    }
    return GLTexture::FORMAT_AUTO;
}

}
