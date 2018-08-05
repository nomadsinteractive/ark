#include "renderer/base/gl_cubemap.h"

#include "core/inf/variable.h"
#include "core/util/bean_utils.h"
#include "core/util/documents.h"
#include "core/util/log.h"

#include "graphics/base/bitmap.h"
#include "graphics/base/image_resource.h"
#include "graphics/base/size.h"

#include "renderer/base/gl_program.h"
#include "renderer/base/gl_recycler.h"
#include "renderer/base/gl_resource_manager.h"
#include "renderer/base/resource_loader_context.h"
#include "renderer/util/gl_contants.h"

namespace ark {

GLCubemap::GLCubemap(const sp<GLRecycler>& recycler, uint32_t width, uint32_t height, const std::vector<sp<Variable<bitmap>>>& bitmaps, GLTexture::Format format)
    : _recycler(recycler), _id(0), _width(width), _height(height), _bitmaps(bitmaps), _format(format)
{
    setTexParameter(static_cast<uint32_t>(GL_TEXTURE_MIN_FILTER), static_cast<int32_t>(GL_LINEAR));
    setTexParameter(static_cast<uint32_t>(GL_TEXTURE_MAG_FILTER), static_cast<int32_t>(GL_LINEAR));
    setTexParameter(static_cast<uint32_t>(GL_TEXTURE_WRAP_S), static_cast<int32_t>(GL_CLAMP_TO_EDGE));
    setTexParameter(static_cast<uint32_t>(GL_TEXTURE_WRAP_T), static_cast<int32_t>(GL_CLAMP_TO_EDGE));
    setTexParameter(static_cast<uint32_t>(GL_TEXTURE_WRAP_R), static_cast<int32_t>(GL_CLAMP_TO_EDGE));
}

GLCubemap::~GLCubemap()
{
    if(_id > 0)
        _recycler->recycle(sp<GLTexture::Recycler>::make(_id));
}

void GLCubemap::setTexParameters(const document& doc)
{
    for(const document& i : doc->children("parameter"))
        _tex_parameters[static_cast<uint32_t>(_gl_constants->getEnum(Documents::ensureAttribute(i, Constants::Attributes::NAME)))] = static_cast<int32_t>(_gl_constants->getEnum(Documents::ensureAttribute(i, Constants::Attributes::VALUE)));
}

void GLCubemap::setTexParameter(uint32_t name, int32_t value)
{
    _tex_parameters[name] = value;
}

void GLCubemap::setTexFormat(GLTexture::Format format)
{
    _format = format;
}

void GLCubemap::prepare(GraphicsContext& /*graphicsContext*/)
{
    if(_id == 0)
        glGenTextures(1, &_id);
    glBindTexture(GL_TEXTURE_CUBE_MAP, _id);

    for(size_t i = 0; i < _bitmaps.size(); ++i)
    {
        const sp<Bitmap> bitmap = _bitmaps[i] ? _bitmaps[i]->val() : sp<Bitmap>::null();
        uint8_t channels = bitmap ? bitmap->channels() : 4;
        GLenum format = GLConstants::getTextureFormat(_format, channels);
        GLenum pixelFormat = bitmap ? GLConstants::getPixelFormat(_format, bitmap) : GL_UNSIGNED_BYTE;
        GLenum internalFormat = bitmap ? GLConstants::getTextureInternalFormat(_format, bitmap) : GL_RGBA8;
        glTexImage2D(static_cast<GLenum>(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i), 0, (GLint) internalFormat, static_cast<int32_t>(_width), static_cast<int32_t>(_height), 0, format, pixelFormat, bitmap ? bitmap->at(0, 0) : nullptr);
        LOGD("GLCubemap Uploaded, id = %d, width = %d, height = %d%s", _id, _width, _height, bitmap ? "" : ", bitmap: nullptr");
    }
    for(const auto i : _tex_parameters)
        glTexParameteri(GL_TEXTURE_CUBE_MAP, static_cast<GLenum>(i.first), static_cast<GLint>(i.second));
}

void GLCubemap::recycle(GraphicsContext& /*graphicsContext*/)
{
    LOGD("Deleting GLCubemap[%d]", _id);
    glDeleteTextures(1, &_id);
    _id = 0;
}

uint32_t GLCubemap::id()
{
    return _id;
}

uint32_t GLCubemap::width() const
{
    return _width;
}

uint32_t GLCubemap::height() const
{
    return _height;
}

void GLCubemap::active(const sp<GLProgram>& program, uint32_t id) const
{
    glActiveTexture(static_cast<GLenum>(GL_TEXTURE0 + id));
    glBindTexture(GL_TEXTURE_CUBE_MAP, _id);

    char uniformName[16] = "u_Texture0";
    uniformName[9] = static_cast<char>('0' + id);
    const GLProgram::Uniform& uTexture = program->getUniform(uniformName);
    uTexture.setUniform1i(id);
}

GLCubemap::BUILDER::BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _resource_loader_context(resourceLoaderContext), _factory(factory), _manifest(manifest), _size(factory.getConcreteClassBuilder<Size>(manifest, Constants::Attributes::SIZE)),
      _format(Documents::getAttribute<GLTexture::Format>(manifest, "format", GLTexture::FORMAT_AUTO))
{
    BeanUtils::split(factory, manifest, Constants::Attributes::SRC, _srcs[0], _srcs[1], _srcs[2], _srcs[3], _srcs[4], _srcs[5]);
}

sp<GLCubemap> GLCubemap::BUILDER::build(const sp<Scope>& args)
{
    std::vector<sp<Variable<bitmap>>> bitmaps;
    const sp<Size> size = _size->build(args);
    for(size_t i = 0; i < 6; ++i)
    {
        const String src = _srcs[i]->build(args);
        bitmaps.push_back(sp<typename Variable<bitmap>::Get>::make(_resource_loader_context->images(), src));
    }
    return sp<GLCubemap>::make(_resource_loader_context->glResourceManager()->recycler(), static_cast<uint32_t>(size->width()), static_cast<uint32_t>(size->height()), bitmaps, _format);
}

}
