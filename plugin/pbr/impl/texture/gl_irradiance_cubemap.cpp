#include "impl/texture/gl_irradiance_cubemap.h"

#include "core/util/math.h"

#include "graphics/base/bitmap.h"
#include "graphics/base/size.h"

#include "renderer/base/gl_resource_manager.h"
#include "renderer/base/gl_shader.h"
#include "renderer/base/gl_texture_default.h"
#include "renderer/base/resource_loader_context.h"
#include "renderer/util/gl_util.h"

#include "platform/gl/gl.h"

#ifndef M_PI
#define M_PI ark::Math::PI
#endif

#ifndef M_SQRT2
#define M_SQRT2 1.4142135623f
#endif

#include "sh.hpp"

namespace ark {

GLIrradianceCubemap::GLIrradianceCubemap(const sp<GLResourceManager>& resourceManager, Format format, Feature features, const sp<GLShader>& shader, const sp<GLTexture>& texture, const sp<Size>& size)
    : GLTexture(resourceManager->recycler(), size, static_cast<uint32_t>(GL_TEXTURE_CUBE_MAP), format, features), _resource_manager(resourceManager), _shader(shader), _texture(texture)
{
}

void GLIrradianceCubemap::doPrepareTexture(GraphicsContext& graphicsContext, uint32_t id)
{
    uint32_t s = static_cast<uint32_t>(_size->width() * 2);
    uint32_t n = s / 2;

    bitmap s1 = bitmap::make(_texture->width(), _texture->height(), _texture->width() * 3 * sizeof(GLfloat), 3);
    bitmap d1 = bitmap::make(n, n, n * 3 * sizeof(GLfloat), 3);

    if(!_texture->id())
        _texture->prepare(graphicsContext);

    glBindTexture(GL_TEXTURE_2D, _texture->id());
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_FLOAT, s1->at(0, 0));
    glBindTexture(GL_TEXTURE_2D, 0);

    if(s1->width() != s || s1->height() != s)
        s1 = s1->resize(s, s);

    sht<float> T1(n, s1->channels());
    T1.S.set(reinterpret_cast<const float*>(s1->at(0, 0)), s);
    T1.ana();
    T1.F.diffuse();
    T1.F.get(reinterpret_cast<float*>(d1->at(0, 0)), n);

    const bitmap s2 = d1->crop(0, 0, n / 2, n / 2);
    const bitmap d2 = bitmap::make(n, n, n * 3 * sizeof(GLfloat), 3);

    sht<float> T2(s2->width(), s2->channels());
    T2.F.set(reinterpret_cast<const float*>(s2->at(0, 0)), s2->width());
    T2.syn();
    T2.S.get(reinterpret_cast<float*>(d2->at(0, 0)));

    GLTextureDefault texture(_resource_manager->recycler(), _size, _format, _features, sp<Variable<bitmap>::Const>::make(d2));
    texture.prepare(graphicsContext);

    GLUtil::renderCubemap(graphicsContext, id, _resource_manager, _shader, texture, n, n);
}

GLIrradianceCubemap::BUILDER::BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _resource_manager(resourceLoaderContext->glResourceManager()), _size(factory.ensureConcreteClassBuilder<Size>(manifest, Constants::Attributes::SIZE)),
      _shader(GLShader::fromDocument(factory, manifest, resourceLoaderContext, "shaders/equirectangular.vert", "shaders/equirectangular.frag")),
      _texture(factory.ensureBuilder<GLTexture>(manifest, Constants::Attributes::TEXTURE)),
      _format(Documents::getAttribute<GLTexture::Format>(manifest, "format", FORMAT_AUTO)),
      _features(Documents::getAttribute<GLTexture::Feature>(manifest, "feature", FEATURE_DEFAULT))
{
}

sp<GLTexture> GLIrradianceCubemap::BUILDER::build(const sp<Scope>& args)
{
    return _resource_manager->createGLResource<GLIrradianceCubemap>(_resource_manager, _format, _features, _shader->build(args), _texture->build(args), _size->build(args));
}

}
