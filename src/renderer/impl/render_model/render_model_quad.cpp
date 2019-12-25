#include "renderer/impl/render_model/render_model_quad.h"

#include "renderer/base/atlas.h"
#include "renderer/base/vertex_stream.h"
#include "renderer/base/drawing_context.h"
#include "renderer/base/drawing_buffer.h"
#include "renderer/base/pipeline_bindings.h"
#include "renderer/base/render_controller.h"
#include "renderer/base/resource_loader_context.h"
#include "renderer/base/shader.h"
#include "renderer/base/shader_bindings.h"

namespace ark {

RenderModelQuad::RenderModelQuad(const RenderController& renderController, const sp<Atlas>& atlas)
    : _atlas(atlas), _index_buffer(renderController.getNamedBuffer(NamedBuffer::NAME_QUADS))
{
}

sp<ShaderBindings> RenderModelQuad::makeShaderBindings(const Shader& shader)
{
    const sp<ShaderBindings> bindings = shader.makeBindings(RENDER_MODE_TRIANGLES, shader.renderController()->makeVertexBuffer(), _index_buffer->buffer());
    bindings->pipelineBindings()->bindSampler(_atlas->texture());
    return bindings;
}

void RenderModelQuad::start(DrawingBuffer& buf, const RenderLayer::Snapshot& snapshot)
{
    buf.vertices().setGrowCapacity(4 * snapshot._items.size());
    buf.setIndices(snapshot._index_buffer);
}

void RenderModelQuad::load(VertexStream& buf, const Renderable::Snapshot& snapshot)
{
    const Atlas::Item& texCoord = _atlas->at(snapshot._type);
    const Rect& bounds = texCoord.bounds();
    const V2& size = texCoord.size();
    float width = static_cast<int32_t>(snapshot._size.x()) == 0 ? size.x() : snapshot._size.x();
    float height = static_cast<int32_t>(snapshot._size.y()) == 0 ? size.y() : snapshot._size.y();

    buf.next();
    buf.writePosition(bounds.left() * width, bounds.top() * height, 0);
    buf.writeTexCoordinate(texCoord.ux(), texCoord.uy());

    buf.next();
    buf.writePosition(bounds.left() * width, bounds.bottom() * height, 0);
    buf.writeTexCoordinate(texCoord.ux(), texCoord.vy());

    buf.next();
    buf.writePosition(bounds.right() * width, bounds.top() * height, 0);
    buf.writeTexCoordinate(texCoord.vx(), texCoord.uy());

    buf.next();
    buf.writePosition(bounds.right() * width, bounds.bottom() * height, 0);
    buf.writeTexCoordinate(texCoord.vx(), texCoord.vy());
}

void RenderModelQuad::postSnapshot(RenderController& renderController, RenderLayer::Snapshot& snapshot)
{
    snapshot._index_buffer = _index_buffer->snapshot(renderController, snapshot._items.size(), snapshot._items.size());
}

Metrics RenderModelQuad::measure(int32_t type)
{
    const Atlas::Item& texCoord = _atlas->at(type);
    return {V3(texCoord.size(), 0), V3(texCoord.size(), 0), {0, 0, 0}};
}

RenderModelQuad::BUILDER::BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _atlas(factory.ensureConcreteClassBuilder<Atlas>(manifest, Constants::Attributes::ATLAS)), _resource_loader_context(resourceLoaderContext)
{
}

sp<RenderModel> RenderModelQuad::BUILDER::build(const Scope& args)
{
    return sp<RenderModelQuad>::make(_resource_loader_context->renderController(), _atlas->build(args));
}

}
