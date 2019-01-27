#include "renderer/opengl/es30/layer/particle_layer.h"

#include "core/base/bean_factory.h"
#include "core/base/object_pool.h"
#include "core/inf/array.h"

#include "graphics/base/matrix.h"

#include "renderer/base/atlas.h"
#include "renderer/base/shader.h"
#include "renderer/base/shader_bindings.h"
#include "renderer/base/resource_manager.h"
#include "renderer/base/resource_loader_context.h"

namespace ark {
namespace gles30 {
/*
ParticleLayer::ParticleLayer(const sp<GLShader>& shader, const sp<Atlas>& atlas, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : Layer(shader->camera(), resourceLoaderContext->memoryPool(), nullptr), _atlas(atlas), _resource_loader_context(resourceLoaderContext),
      _shader_bindings(sp<ShaderBindings>::make(resourceLoaderContext->glResourceManager(), shader)),
      _index_buffer(GLIndexBuffers::makeGLBufferSnapshot(resourceLoaderContext->glResourceManager(), GLBuffer::NAME_QUADS, 1)),
      _transform_array_buffer(resourceLoaderContext->glResourceManager()->makeDynamicArrayBuffer())
{
//    _shader_bindings->setInstancedArrayBuffer(1, _transform_array_buffer);
}

sp<RenderCommand> ParticleLayer::render(const Layer::Snapshot& renderContext, float x, float y)
{
    if(renderContext._items.size() == 0)
        return nullptr;

    const array<uint8_t> pos = _memory_pool.allocate((4 * 3 + 4) * 4);
    float* pPos = reinterpret_cast<float*>(pos->buf());
    uint16_t* pTex;
    const Atlas::Item& texCoord = _atlas->at(renderContext._items.begin()->_type);

    pPos[0] = -0.5f;
    pPos[1] = -0.5f;
    pPos[2] = 0;
    pTex = reinterpret_cast<uint16_t*>(pPos + 3);
    pTex[0] = texCoord.left();
    pTex[1] = texCoord.top();

    pPos[4] = -0.5f;
    pPos[5] = 0.5f;
    pPos[6] = 0;
    pTex = reinterpret_cast<uint16_t*>(pPos + 7);
    pTex[0] = texCoord.left();
    pTex[1] = texCoord.bottom();

    pPos[8] = 0.5f;
    pPos[9] = -0.5f;
    pPos[10] = 0;
    pTex = reinterpret_cast<uint16_t*>(pPos + 11);
    pTex[0] = texCoord.right();
    pTex[1] = texCoord.top();

    pPos[12] = 0.5f;
    pPos[13] = 0.5f;
    pPos[14] = 0;
    pTex = reinterpret_cast<uint16_t*>(pPos + 15);
    pTex[0] = texCoord.right();
    pTex[1] = texCoord.bottom();

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
        transform.translate += V(tx, ty);
        transform.scale *= V(width, height);
        const Matrix uTransform = transform.toMatrix();
        memcpy(pTransform, &uTransform, sizeof(uTransform));
        pTransform += 16;
    }

    GLDrawingContext dc(_shader_bindings, renderContext._camera, _shader_bindings->arrayBuffer().snapshot(_resource_loader_context->objectPool()->obtain<GLUploader::ByteArray>(pos)), _index_buffer, GL_TRIANGLES);
//    dc._instanced_array_buffers[1] = _transform_array_buffer.snapshot(_resource_loader_context->objectPool()->obtain<GLUploader::ByteArray>(transform));
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
*/
}
}
