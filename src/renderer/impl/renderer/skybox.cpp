#include "renderer/impl/renderer/skybox.h"

#include "core/base/api.h"
#include "core/impl/array/preallocated_array.h"

#include "graphics/base/render_request.h"
#include "graphics/base/size.h"

#include "renderer/base/gl_buffer.h"
#include "renderer/base/gl_cubemap.h"
#include "renderer/base/gl_drawing_context.h"
#include "renderer/base/gl_resource_manager.h"
#include "renderer/base/gl_shader.h"
#include "renderer/base/gl_shader_bindings.h"
#include "renderer/base/gl_snippet_delegate.h"
#include "renderer/base/resource_loader_context.h"
#include "renderer/impl/gl_snippet/gl_snippet_active_cubemap.h"
#include "renderer/util/gl_index_buffers.h"

namespace ark {

namespace {

class RenderCommandSkybox : public RenderCommand {
public:
    RenderCommandSkybox(GLDrawingContext context, const sp<GLShader>& shader)
        : _context(std::move(context)), _shader(shader) {
    }

    virtual void draw(GraphicsContext& graphicsContext) override {
        _shader->use(graphicsContext);
        _shader->bindUniforms(graphicsContext);
        _context.preDraw(graphicsContext, _shader);
        glDrawElements(_context._mode, _context._count, GLIndexType, 0);
        _context.postDraw(graphicsContext);
    }

private:
    GLDrawingContext _context;
    sp<GLShader> _shader;
};

}


Skybox::Skybox(const sp<Size>& size, const sp<GLShader>& shader, const sp<GLCubemap>& cubemap, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _size(size), _resource_manager(resourceLoaderContext->glResourceManager()), _shader(shader),
      _shader_bindings(sp<GLShaderBindings>::make(_resource_manager, shader, _resource_manager->makeGLBuffer(sp<GLBuffer::ByteArrayUploader>::make(getArrayBuffer()), GL_ARRAY_BUFFER, GL_STATIC_DRAW))),
      _object_pool(resourceLoaderContext->objectPool())
{
    _shader_bindings->snippet()->link<GLSnippetActiveCubemap>(cubemap);
}

void Skybox::render(RenderRequest& renderRequest, float x, float y)
{
    const GLBuffer::Snapshot indexBuffer = GLIndexBuffers::makeGLBufferSnapshot(_resource_manager, GLBuffer::NAME_QUADS, 6);
    renderRequest.addRequest(_object_pool->obtain<RenderCommandSkybox>(GLDrawingContext(_shader_bindings, _shader_bindings->arrayBuffer().snapshot(), indexBuffer, GL_TRIANGLES), _shader));
}

const sp<Size>& Skybox::size()
{
    return _size;
}

bytearray Skybox::getArrayBuffer() const
{
    static float vertices[] = {
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,

         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f
    };
    return sp<PreallocatedArray<uint8_t>>::make(reinterpret_cast<uint8_t*>(vertices), sizeof(vertices));
}

Skybox::BUILDER::BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _resource_loader_context(resourceLoaderContext), _size(factory.ensureBuilder<Size>(manifest, Constants::Attributes::SIZE)),
      _shader(GLShader::fromDocument(factory, manifest, resourceLoaderContext, "shaders/skybox.vert", "shaders/skybox.frag")),
      _cubemap(factory.ensureConcreteClassBuilder<GLCubemap>(manifest, Constants::Attributes::CUBEMAP))
{
}

sp<Renderer> Skybox::BUILDER::build(const sp<Scope>& args)
{
    const sp<Size> size = _size->build(args);
    const sp<GLShader> shader = _shader->build(args);
    return sp<Skybox>::make(size, shader, _cubemap->build(args), _resource_loader_context);
}

}
