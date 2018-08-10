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
#include "renderer/util/gl_util.h"

namespace ark {

GLTexture::GLTexture(const sp<GLRecycler>& recycler, const sp<Size>& size, uint32_t target, Format format, Feature features)
    : _recycler(recycler), _id(0), _size(size), _target(target), _format(format), _features(features)
{
    setTexParameter(static_cast<uint32_t>(GL_TEXTURE_MIN_FILTER), static_cast<int32_t>((_features & GLTexture::FEATURE_MIPMAPS) ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR));
    setTexParameter(static_cast<uint32_t>(GL_TEXTURE_MAG_FILTER), static_cast<int32_t>(GL_LINEAR));
    setTexParameter(static_cast<uint32_t>(GL_TEXTURE_WRAP_S), static_cast<int32_t>(GL_CLAMP_TO_EDGE));
    setTexParameter(static_cast<uint32_t>(GL_TEXTURE_WRAP_T), static_cast<int32_t>(GL_CLAMP_TO_EDGE));
    setTexParameter(static_cast<uint32_t>(GL_TEXTURE_WRAP_R), static_cast<int32_t>(GL_CLAMP_TO_EDGE));
}

GLTexture::~GLTexture()
{
    if(_id > 0)
        _recycler->recycle(sp<Recycler>::make(_id));
}

void GLTexture::setTexParameters(const document& doc)
{
    for(const document& i : doc->children("parameter"))
        _tex_parameters[static_cast<uint32_t>(GLUtil::getEnum(Documents::ensureAttribute(i, Constants::Attributes::NAME)))] = static_cast<int32_t>(GLUtil::getEnum(Documents::ensureAttribute(i, Constants::Attributes::VALUE)));
}

void GLTexture::setTexParameter(uint32_t name, int32_t value)
{
    _tex_parameters[name] = value;
}

void GLTexture::prepare(GraphicsContext& graphicsContext)
{
    if(_id == 0)
        glGenTextures(1, &_id);

    glBindTexture(static_cast<GLenum>(_target), _id);
    doPrepareTexture(graphicsContext, _id);

    if(_features & FEATURE_MIPMAPS)
        glGenerateMipmap(static_cast<GLenum>(_target));

    for(const auto i : _tex_parameters)
        glTexParameteri(static_cast<GLenum>(_target), static_cast<GLenum>(i.first), static_cast<GLint>(i.second));
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

int32_t GLTexture::width() const
{
    return static_cast<int32_t>(_size->width());
}

int32_t GLTexture::height() const
{
    return static_cast<int32_t>(_size->height());
}

int32_t GLTexture::depth() const
{
    return static_cast<int32_t>(_size->depth());
}

const sp<Size>& GLTexture::size() const
{
    return _size;
}

void GLTexture::active(GLProgram& program, uint32_t name)
{
    glActiveTexture(static_cast<GLenum>(GL_TEXTURE0 + name));
    glBindTexture(static_cast<GLenum>(_target), _id);

    char uniformName[16] = "u_Texture0";
    uniformName[9] = static_cast<char>('0' + name);
    const GLProgram::Uniform& uTexture = program.getUniform(uniformName);
    uTexture.setUniform1i(name);
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
            else
                DFATAL("Unknow texture format: %s", i.c_str());
        }
        return format;
    }
    return GLTexture::FORMAT_AUTO;
}

template<> ARK_API GLTexture::Feature Conversions::to<String, GLTexture::Feature>(const String& str)
{
    if(str)
    {
        GLTexture::Feature feature = GLTexture::FEATURE_DEFAULT;
        for(const String& i : str.toLower().split('|'))
        {
            if(i == "mipmaps")
                feature = static_cast<GLTexture::Feature>(feature | GLTexture::FEATURE_MIPMAPS);
            else
                DFATAL("Unknow texture feature: %s", i.c_str());
        }
        return feature;
    }
    return GLTexture::FEATURE_DEFAULT;
}

}
