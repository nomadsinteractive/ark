#include "renderer/gles30/impl/layer/particle_layer.h"

#include "core/base/bean_factory.h"
#include "core/inf/array.h"

#include "graphics/base/matrix.h"

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
      _position_array_buffer(resourceLoaderContext->glResourceManager()->createDynamicArrayBuffer()),
      _transform_array_buffer(resourceLoaderContext->glResourceManager()->createDynamicArrayBuffer())
{
}

sp<RenderCommand> ParticleLayer::render(const LayerContext::Snapshot& renderContext, float x, float y)
{
    if(renderContext._items.size() == 0)
        return nullptr;

    const array<uint8_t> pos = _memory_pool.allocate(4 * 3 * 4);
    float* pPos = reinterpret_cast<float*>(pos->buf());

    pPos[0] = -0.5f;
    pPos[1] = -0.5f;
    pPos[2] = 0;

    pPos[3] = -0.5f;
    pPos[4] = 0.5f;
    pPos[5] = 0;

    pPos[6] = 0.5f;
    pPos[7] = -0.5f;
    pPos[8] = 0;

    pPos[9] = 0.5f;
    pPos[10] = 0.5f;
    pPos[11] = 0;

    const array<uint8_t> buf = _memory_pool.allocate(renderContext._items.size() * 8 * 2);
    uint16_t* pBuf = reinterpret_cast<uint16_t*>(buf->buf());
    const array<uint8_t> transform = _memory_pool.allocate(renderContext._items.size() * 4 * 4 * 4);
    float* pTransform = reinterpret_cast<float*>(transform->buf());
    for(const RenderObject::Snapshot& i : renderContext._items)
    {
        const Atlas::Item& texCoord = _atlas->at(i._type);
        Transform::Snapshot transform = i._transform;
        const V& position = i._position;
        float w = i._size.x();
        float h = i._size.y();
        float width = w == 0 ? texCoord.width() : w;
        float height = h == 0 ? texCoord.height() : h;
        float tx = position.x() + x;
        float ty = position.y() + y;

        pBuf[0] = texCoord.left();
        pBuf[1] = texCoord.top();
        pBuf[2] = texCoord.left();
        pBuf[3] = texCoord.bottom();
        pBuf[4] = texCoord.right();
        pBuf[5] = texCoord.top();
        pBuf[6] = texCoord.right();
        pBuf[7] = texCoord.bottom();
        transform.scale = transform.scale * V(width, height);

        Matrix uTransform = transform.toMatrix();
        uTransform.translate(tx, ty, 0.0f);
        memcpy(pTransform, &uTransform, sizeof(uTransform));

        pBuf += 8;
        pTransform += 16;
    }

    GLDrawingContext dc(_shader_bindings, _shader_bindings->arrayBuffer().snapshot(buf), _index_buffer, GL_TRIANGLES);
    dc._instanced_array_buffers[1] = _transform_array_buffer.snapshot(transform);
    dc._instanced_array_buffers[1000] = _position_array_buffer.snapshot(pos);
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
