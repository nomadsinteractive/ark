#include "renderer/impl/render_model/render_model_quad.h"

#include "renderer/base/atlas.h"
#include "renderer/base/drawing_context.h"
#include "renderer/base/drawing_buffer.h"
#include "renderer/base/pipeline_bindings.h"
#include "renderer/base/render_controller.h"
#include "renderer/base/resource_loader_context.h"
#include "renderer/base/shader.h"
#include "renderer/base/shader_bindings.h"

namespace ark {

GLModelQuad::GLModelQuad(const RenderController& renderController, const sp<Atlas>& atlas)
    : _atlas(atlas), _index_buffer(renderController.getNamedBuffer(NamedBuffer::NAME_QUADS))
{
}

sp<ShaderBindings> GLModelQuad::makeShaderBindings(const Shader& shader)
{
    const sp<ShaderBindings> bindings = shader.makeBindings(RENDER_MODE_TRIANGLES, shader.renderController()->makeVertexBuffer(), _index_buffer->buffer());
    bindings->pipelineBindings()->bindSampler(_atlas->texture());
    return bindings;
}

void GLModelQuad::start(DrawingBuffer& buf, const RenderLayer::Snapshot& snapshot)
{
    buf.vertices().setGrowCapacity(4 * snapshot._items.size());
    buf.setIndices(snapshot._index_buffer);
}

void GLModelQuad::load(DrawingBuffer& buf, const RenderObject::Snapshot& snapshot)
{
    const Atlas::Item& texCoord = _atlas->at(snapshot._type);
    const V2& pivot = texCoord.pivot();
    float width = static_cast<int32_t>(snapshot._size.x()) == 0 ? texCoord.width() : snapshot._size.x();
    float height = static_cast<int32_t>(snapshot._size.y()) == 0 ? texCoord.height() : snapshot._size.y();

    buf.nextVertex();
    buf.writePosition(0 - pivot.x() * width, 0 - pivot.y() * height, 0);
    buf.writeTexCoordinate(texCoord.left(), texCoord.top());

    buf.nextVertex();
    buf.writePosition(0 - pivot.x() * width, height - pivot.y() * height, 0);
    buf.writeTexCoordinate(texCoord.left(), texCoord.bottom());

    buf.nextVertex();
    buf.writePosition(width - pivot.x() * width, 0 - pivot.y() * height, 0);
    buf.writeTexCoordinate(texCoord.right(), texCoord.top());

    buf.nextVertex();
    buf.writePosition(width - pivot.x() * width, height - pivot.y() * height, 0);
    buf.writeTexCoordinate(texCoord.right(), texCoord.bottom());
}

void GLModelQuad::postSnapshot(RenderController& renderController, RenderLayer::Snapshot& snapshot)
{
    snapshot._index_buffer = _index_buffer->snapshot(renderController, snapshot._items.size());
}

Metrics GLModelQuad::measure(int32_t type)
{
    const Atlas::Item& texCoord = _atlas->at(type);
    return {texCoord.size(), texCoord.size(), {0, 0}};
}

GLModelQuad::BUILDER::BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _atlas(factory.ensureBuilder<Atlas>(manifest, Constants::Attributes::ATLAS)), _resource_loader_context(resourceLoaderContext)
{
}

sp<RenderModel> GLModelQuad::BUILDER::build(const sp<Scope>& args)
{
    return sp<GLModelQuad>::make(_resource_loader_context->renderController(), _atlas->build(args));
}

}
