#include "renderer/impl/renderer/skybox.h"

#include "core/base/api.h"

#include "graphics/base/render_request.h"
#include "graphics/base/size.h"

#include "renderer/base/buffer.h"
#include "renderer/base/drawing_context.h"
#include "renderer/base/graphics_context.h"
#include "renderer/base/pipeline_input.h"
#include "renderer/base/render_controller.h"
#include "renderer/base/shader.h"
#include "renderer/base/shader_bindings.h"
#include "renderer/base/snippet_delegate.h"
#include "renderer/base/resource_loader_context.h"

#include "renderer/opengl/base/gl_cubemap.h"
#include "renderer/opengl/util/gl_util.h"

namespace ark {

Skybox::Skybox(const sp<Size>& size, const sp<Shader>& shader, const sp<Texture>& texture, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _size(size), _shader(shader),
      _shader_bindings(sp<ShaderBindings>::make(RenderModel::RENDER_MODE_TRIANGLES, resourceLoaderContext->renderController(), shader->pipelineLayout(), resourceLoaderContext->renderController()->makeVertexBuffer(Buffer::USAGE_STATIC, sp<ByteArrayUploader>::make(GLUtil::makeUnitCubeVertices())), resourceLoaderContext->renderController()->makeIndexBuffer(Buffer::USAGE_STATIC))),
      _memory_pool(resourceLoaderContext->memoryPool()), _object_pool(resourceLoaderContext->objectPool()),
      _index_buffer(resourceLoaderContext->renderController()->getNamedBuffer(NamedBuffer::NAME_QUADS)->snapshot(resourceLoaderContext->renderController(), 6))
{
    _shader_bindings->bindSampler(texture);
}

void Skybox::render(RenderRequest& renderRequest, float x, float y)
{
    DrawingContext drawingContext(_shader, _shader_bindings, _shader->snapshot(_memory_pool), _shader_bindings->vertexBuffer().snapshot(), _index_buffer, 0);
    renderRequest.addRequest(drawingContext.toRenderCommand(_object_pool));
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
