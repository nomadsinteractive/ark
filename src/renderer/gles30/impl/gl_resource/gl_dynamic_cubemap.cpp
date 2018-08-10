#include "renderer/gles30/impl/gl_resource/gl_dynamic_cubemap.h"

#include "core/impl/array/preallocated_array.h"
#include "core/util/log.h"
#include "core/util/math.h"

#include "graphics/base/matrix.h"
#include "graphics/base/size.h"
#include "graphics/base/v3.h"

#include "renderer/base/gl_buffer.h"
#include "renderer/base/gl_drawing_context.h"
#include "renderer/base/gl_program.h"
#include "renderer/base/gl_recycler.h"
#include "renderer/base/gl_resource_manager.h"
#include "renderer/base/gl_shader.h"
#include "renderer/base/gl_texture.h"
#include "renderer/base/resource_loader_context.h"
#include "renderer/util/gl_index_buffers.h"
#include "renderer/util/gl_debug.h"

#include "platform/gl/gl.h"

namespace ark {

static bytearray getArrayBuffer()
{
    static float vertices[] = {
        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,

        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f
    };
    return sp<PreallocatedArray<uint8_t>>::make(reinterpret_cast<uint8_t*>(vertices), sizeof(vertices));
}

extern uint32_t g_GLViewportWidth;
extern uint32_t g_GLViewportHeight;

GLDynamicCubemap::GLDynamicCubemap(const sp<GLResourceManager>& resourceManager, Format format, Feature features, const sp<GLShader>& shader, const sp<GLTexture>& texture, const sp<Size>& size)
    : GLTexture(resourceManager->recycler(), size, static_cast<uint32_t>(GL_TEXTURE_CUBE_MAP), format, features), _resource_manager(resourceManager), _shader(shader), _texture(texture)
{
}

void GLDynamicCubemap::doPrepareTexture(GraphicsContext& graphicsContext, uint32_t id)
{
    uint32_t captureFBO, captureRBO;
    glGenFramebuffers(1, &captureFBO);
    glGenRenderbuffers(1, &captureRBO);

    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width(), height());
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);

    for (uint32_t i = 0; i < 6; ++i)
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, width(), height(), 0, GL_RGB, GL_FLOAT, nullptr);

    const Matrix captureProjection = Matrix::perspective(Math::radians(90.0f), 1.0f, 0.1f, 10.0f);
    const Matrix captureViews[] = {
        Matrix::lookAt(V3(0.0f, 0.0f, 0.0f), V3( 1.0f,  0.0f,  0.0f), V3(0.0f, -1.0f,  0.0f)),
        Matrix::lookAt(V3(0.0f, 0.0f, 0.0f), V3(-1.0f,  0.0f,  0.0f), V3(0.0f, -1.0f,  0.0f)),
        Matrix::lookAt(V3(0.0f, 0.0f, 0.0f), V3( 0.0f,  1.0f,  0.0f), V3(0.0f,  0.0f,  1.0f)),
        Matrix::lookAt(V3(0.0f, 0.0f, 0.0f), V3( 0.0f, -1.0f,  0.0f), V3(0.0f,  0.0f, -1.0f)),
        Matrix::lookAt(V3(0.0f, 0.0f, 0.0f), V3( 0.0f,  0.0f,  1.0f), V3(0.0f, -1.0f,  0.0f)),
        Matrix::lookAt(V3(0.0f, 0.0f, 0.0f), V3( 0.0f,  0.0f, -1.0f), V3(0.0f, -1.0f,  0.0f))
    };

    _shader->use(graphicsContext);
    _shader->program()->getUniform("u_Projection").setUniformMatrix4fv(1, GL_FALSE, captureProjection.value());
    _texture->prepare(graphicsContext);
    _texture->active(_shader->program(), 0);

    uint32_t vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    GLBuffer arrayBuffer(_recycler, sp<GLBuffer::ArrayUploader<uint8_t>>::make(getArrayBuffer()), GL_ARRAY_BUFFER, GL_STATIC_DRAW);
    arrayBuffer.prepare(graphicsContext);
    glBindBuffer(GL_ARRAY_BUFFER, arrayBuffer.id());
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 12, 0);

    const GLBuffer::Snapshot indexBuffer = GLIndexBuffers::makeGLBufferSnapshot(_resource_manager, GLBuffer::NAME_QUADS, 6);
    indexBuffer.prepare(graphicsContext);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer.id());

    glViewport(0, 0, width(), height());
    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    for (uint32_t i = 0; i < 6; ++i)
    {
        _shader->program()->getUniform("u_View").setUniformMatrix4fv(1, GL_FALSE, captureViews[i].value());
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, id, 0);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDrawElements(GL_TRIANGLES, 36, GLIndexType, 0);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindVertexArray(0);

    glDeleteVertexArrays(1, &vao);
    glDeleteFramebuffers(1, &captureFBO);
    glDeleteRenderbuffers(1, &captureRBO);

    glViewport(0, 0, g_GLViewportWidth, g_GLViewportHeight);
}

GLDynamicCubemap::BUILDER::BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _resource_manager(resourceLoaderContext->glResourceManager()), _size(factory.ensureConcreteClassBuilder<Size>(manifest, Constants::Attributes::SIZE)),
      _shader(GLShader::fromDocument(factory, manifest, resourceLoaderContext, "shaders/equirectangular.vert", "shaders/equirectangular.frag")),
      _texture(factory.ensureBuilder<GLTexture>(manifest, Constants::Attributes::TEXTURE)),
      _format(Documents::getAttribute<GLTexture::Format>(manifest, "format", FORMAT_AUTO)),
      _features(Documents::getAttribute<GLTexture::Feature>(manifest, "feature", FEATURE_DEFAULT))

{
}

sp<GLTexture> GLDynamicCubemap::BUILDER::build(const sp<Scope>& args)
{
    return _resource_manager->createGLResource<GLDynamicCubemap>(_resource_manager, _format, _features, _shader->build(args), _texture->build(args), _size->build(args));
}

}
