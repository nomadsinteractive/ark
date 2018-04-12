#include "renderer/gles30/impl/layer/particle_layer.h"

#include "core/base/bean_factory.h"
#include "core/inf/array.h"

#include "renderer/base/atlas.h"

#include "renderer/base/gl_shader.h"
#include "renderer/base/gl_shader_bindings.h"
#include "renderer/base/gl_resource_manager.h"
#include "renderer/base/resource_loader_context.h"
#include "renderer/impl/render_command/draw_elements_instanced.h"

namespace ark {
namespace gles30 {

ParticleLayer::ParticleLayer(const sp<GLShader>& shader, const sp<Atlas>& atlas, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : Layer(resourceLoaderContext->memoryPool()), _atlas(atlas),
      _shader_bindings(sp<GLShaderBindings>::make(shader, resourceLoaderContext->glResourceManager()->createDynamicArrayBuffer())),
      _index_buffer(resourceLoaderContext->glResourceManager()->getGLIndexBuffer(GLResourceManager::BUFFER_NAME_TRANGLES, 6)),
      _transform_array_buffer(resourceLoaderContext->glResourceManager()->createDynamicArrayBuffer())
{
}

sp<RenderCommand> ParticleLayer::render(const LayerContext::Snapshot& renderContext, float x, float y)
{
    const array<uint8_t> buf = _memory_pool.allocate(renderContext._items.size() * 4);
    uint16_t* pBuf = reinterpret_cast<uint16_t*>(buf->buf());
    const array<uint8_t> transform = _memory_pool.allocate(renderContext._items.size() * 4 * 4 * 4);
    float* pTransform = reinterpret_cast<float*>(transform->buf());

    for(const RenderObject::Snapshot& i : renderContext._items)
    {

    }

    GLDrawingContext dc(_shader_bindings, _shader_bindings->arrayBuffer().snapshot(buf), _index_buffer, GL_TRIANGLES);
    dc._instanced_array_buffers[1] = _transform_array_buffer.snapshot(transform);
    return sp<DrawElementsInstanced>::make(dc, _shader_bindings->shader(), renderContext._items.size());
}

ParticleLayer::BUILDER::BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _resource_loader_context(resourceLoaderContext), _atlas(factory.ensureBuilder<Atlas>(manifest)),
      _shader(GLShader::fromDocument(factory, manifest, resourceLoaderContext)) {
}

sp<Layer> ParticleLayer::BUILDER::build(const sp<Scope>& args)
{
    const sp<Atlas> atlas = _atlas->build(args);
    return sp<ParticleLayer>::make(_shader->build(args), atlas, _resource_loader_context);
}

}
}
