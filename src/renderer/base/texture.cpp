#include "renderer/base/texture.h"

#include "core/inf/array.h"
#include "core/inf/variable.h"
#include "core/util/conversions.h"
#include "core/util/documents.h"
#include "core/util/log.h"

#include "graphics/base/bitmap.h"
#include "graphics/base/size.h"

#include "renderer/base/gl_recycler.h"
#include "renderer/base/gl_resource_manager.h"
#include "renderer/base/gl_texture_loader.h"
#include "renderer/base/graphics_context.h"
#include "renderer/base/resource_loader_context.h"
#include "renderer/opengl/util/gl_util.h"

namespace ark {

Texture::Texture(const sp<GLRecycler>& recycler, const sp<Size>& size, uint32_t target, Format format, Feature features)
    : _recycler(recycler), _size(size), _target(target), _format(format), _features(features), _id(0)
{
    setTexParameter(static_cast<uint32_t>(GL_TEXTURE_MIN_FILTER), static_cast<int32_t>((_features & Texture::FEATURE_MIPMAPS) ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR));
    setTexParameter(static_cast<uint32_t>(GL_TEXTURE_MAG_FILTER), static_cast<int32_t>(GL_LINEAR));
    setTexParameter(static_cast<uint32_t>(GL_TEXTURE_WRAP_S), static_cast<int32_t>(GL_CLAMP_TO_EDGE));
    setTexParameter(static_cast<uint32_t>(GL_TEXTURE_WRAP_T), static_cast<int32_t>(GL_CLAMP_TO_EDGE));
    setTexParameter(static_cast<uint32_t>(GL_TEXTURE_WRAP_R), static_cast<int32_t>(GL_CLAMP_TO_EDGE));
}

Texture::Texture(const sp<GLRecycler>& recycler, const sp<Size>& size, uint32_t target, const document& manifest)
    : Texture(recycler, size, target, Documents::getAttribute<Texture::Format>(manifest, "format", FORMAT_AUTO), Documents::getAttribute<Texture::Feature>(manifest, "feature", FEATURE_DEFAULT))
{
    setTexParameters(manifest);
}

Texture::~Texture()
{
    if(_id > 0)
        _recycler->recycle(sp<Recycler>::make(_id));
}

void Texture::setTexParameters(const document& doc)
{
    for(const document& i : doc->children("parameter"))
        _tex_parameters[static_cast<uint32_t>(GLUtil::getEnum(Documents::ensureAttribute(i, Constants::Attributes::NAME)))] = static_cast<int32_t>(GLUtil::getEnum(Documents::ensureAttribute(i, Constants::Attributes::VALUE)));
}

void Texture::setTexParameter(uint32_t name, int32_t value)
{
    _tex_parameters[name] = value;
}

void Texture::prepare(GraphicsContext& graphicsContext)
{
    if(_id == 0)
        glGenTextures(1, &_id);

    glBindTexture(static_cast<GLenum>(_target), _id);
    doPrepareTexture(graphicsContext, _id);

    if(_features & FEATURE_MIPMAPS)
        glGenerateMipmap(static_cast<GLenum>(_target));

    for(const auto& i : _tex_parameters)
        glTexParameteri(static_cast<GLenum>(_target), static_cast<GLenum>(i.first), static_cast<GLint>(i.second));
}

void Texture::recycle(GraphicsContext& /*graphicsContext*/)
{
    LOGD("Deleting GLTexture[%d]", _id);
    glDeleteTextures(1, &_id);
    _id = 0;
}

uint32_t Texture::id()
{
    return _id;
}

int32_t Texture::width() const
{
    return static_cast<int32_t>(_size->width());
}

int32_t Texture::height() const
{
    return static_cast<int32_t>(_size->height());
}

int32_t Texture::depth() const
{
    return static_cast<int32_t>(_size->depth());
}

const sp<Size>& Texture::size() const
{
    return _size;
}

uint32_t Texture::target() const
{
    return _target;
}

void Texture::active(Pipeline& pipeline, uint32_t name)
{
    pipeline.activeTexture(*this, _target, name);
}

Texture::Recycler::Recycler(uint32_t id)
    : _id(id)
{
}

uint32_t Texture::Recycler::id()
{
    return _id;
}

void Texture::Recycler::prepare(GraphicsContext&)
{
}

void Texture::Recycler::recycle(GraphicsContext&)
{
    LOGD("Deleting GLTexture[%d]", _id);
    glDeleteTextures(1, &_id);
    _id = 0;
}

template<> ARK_API Texture::Format Conversions::to<String, Texture::Format>(const String& str)
{
    if(str)
    {
        Texture::Format format = Texture::FORMAT_R;
        for(const String& i : str.toLower().split('|'))
        {
            if(i == "r")
                format = static_cast<Texture::Format>(format | Texture::FORMAT_R);
            else if(i == "rg")
                format = static_cast<Texture::Format>(format | Texture::FORMAT_RG);
            else if(i == "rgb")
                format = static_cast<Texture::Format>(format | Texture::FORMAT_RGB);
            else if(i == "rgba")
                format = static_cast<Texture::Format>(format | Texture::FORMAT_RGBA);
            else if(i == "signed")
                format = static_cast<Texture::Format>(format | Texture::FORMAT_SIGNED);
            else
                DFATAL("Unknow texture format: %s", i.c_str());
        }
        return format;
    }
    return Texture::FORMAT_AUTO;
}

template<> ARK_API Texture::Feature Conversions::to<String, Texture::Feature>(const String& str)
{
    if(str)
    {
        Texture::Feature feature = Texture::FEATURE_DEFAULT;
        for(const String& i : str.toLower().split('|'))
        {
            if(i == "mipmaps")
                feature = static_cast<Texture::Feature>(feature | Texture::FEATURE_MIPMAPS);
            else
                DFATAL("Unknow texture feature: %s", i.c_str());
        }
        return feature;
    }
    return Texture::FEATURE_DEFAULT;
}

}
