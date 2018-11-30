#include "renderer/impl/renderer/skybox.h"

#include "core/base/api.h"

#include "graphics/base/render_request.h"
#include "graphics/base/size.h"

#include "renderer/base/gl_buffer.h"
#include "renderer/opengl/base/gl_cubemap.h"
#include "renderer/base/drawing_context.h"
#include "renderer/base/graphics_context.h"
#include "renderer/base/gl_resource_manager.h"
#include "renderer/base/shader.h"
#include "renderer/base/shader_bindings.h"
#include "renderer/base/gl_snippet_delegate.h"
#include "renderer/base/resource_loader_context.h"
#include "renderer/opengl/util/gl_index_buffers.h"
#include "renderer/opengl/util/gl_util.h"

namespace ark {

namespace {

class RenderCommandSkybox : public RenderCommand {
public:
    RenderCommandSkybox(DrawingContext context, const sp<Shader>& shader, const Matrix& view, const Matrix& projection)
        : _context(std::move(context)), _shader(shader), _view(view), _projection(projection) {
    }

    virtual void draw(GraphicsContext& graphicsContext) override {
        _shader->use(graphicsContext);
        _shader->bindUniforms(graphicsContext);
        _context.preDraw(graphicsContext, _shader);

        GLPipeline* pipeline = static_cast<GLPipeline*>(_shader->pipeline().get());
        pipeline->glUpdateMatrix(graphicsContext, "u_View", _view);
        pipeline->glUpdateMatrix(graphicsContext, "u_Projection", _projection);

        glDrawElements(GL_TRIANGLES, _context._count, GLIndexType, nullptr);
        _context.postDraw(graphicsContext);
    }

private:
    DrawingContext _context;
    sp<Shader> _shader;
    Matrix _view;
    Matrix _projection;
};

}


Skybox::Skybox(const sp<Size>& size, const sp<Shader>& shader, const sp<Texture>& texture, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _size(size), _resource_manager(resourceLoaderContext->resourceManager()), _shader(shader), _index_buffer(GLIndexBuffers::makeGLBufferSnapshot(_resource_manager, GLBuffer::NAME_QUADS, 6)),
      _shader_bindings(sp<ShaderBindings>::make(_resource_manager, shader, _resource_manager->makeGLBuffer(sp<GLBuffer::ByteArrayUploader>::make(GLUtil::makeUnitCubeVertices()), GL_ARRAY_BUFFER, GL_STATIC_DRAW))),
      _object_pool(resourceLoaderContext->objectPool())
{
    _shader_bindings->bindGLTexture(texture);
}

void Skybox::render(RenderRequest& renderRequest, float x, float y)
{
    const Matrix view = _shader->camera()->view();
    const Matrix projection = _shader->camera()->projection();
    renderRequest.addRequest(_object_pool->obtain<RenderCommandSkybox>(DrawingContext(_shader_bindings, _shader->camera()->snapshop(), _shader_bindings->arrayBuffer().snapshot(), _index_buffer), _shader, view, projection));
}

const SafePtr<Size>& Skybox::size()
{
    return _size;
}

Skybox::BUILDER::BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _resource_loader_context(resourceLoaderContext), _size(factory.ensureBuilder<Size>(manifest, Constants::Attributes::SIZE)),
      _shader(Shader::fromDocument(factory, manifest, resourceLoaderContext, "shaders/skybox.vert", "shaders/skybox.frag")),
      _texture(factory.ensureBuilder<Texture>(manifest, Constants::Attributes::TEXTURE))
{
}

sp<Renderer> Skybox::BUILDER::build(const sp<Scope>& args)
{
    const sp<Size> size = _size->build(args);
    return sp<Skybox>::make(size, _shader->build(args), _texture->build(args), _resource_loader_context);
}

}
