#include "renderer/impl/render_model/render_model_point.h"

#include "renderer/base/atlas.h"
#include "renderer/base/drawing_buffer.h"
#include "renderer/base/pipeline_bindings.h"
#include "renderer/base/resource_loader_context.h"
#include "renderer/base/shader.h"
#include "renderer/base/shader_bindings.h"

namespace ark {

RenderModelPoint::RenderModelPoint(const RenderController& renderController, const sp<Atlas>& atlas)
    : _atlas(atlas), _index_buffer(renderController.getNamedBuffer(NamedBuffer::NAME_POINTS))
{
}

sp<ShaderBindings> RenderModelPoint::makeShaderBindings(const Shader& shader)
{
    const sp<ShaderBindings> bindings = shader.makeBindings(RENDER_MODE_POINTS, shader.renderController()->makeVertexBuffer(), _index_buffer->buffer());
    bindings->pipelineBindings()->bindSampler(_atlas->texture());
    return bindings;
}

void RenderModelPoint::postSnapshot(RenderController& renderController, RenderLayer::Snapshot& snapshot)
{
    snapshot._index_buffer = _index_buffer->snapshot(renderController, snapshot._items.size(), snapshot._items.size());
}

void RenderModelPoint::start(DrawingBuffer& buf, const RenderLayer::Snapshot& snapshot)
{
    buf.vertices().setGrowCapacity(snapshot._items.size());
    buf.setIndices(snapshot._index_buffer);
}

void RenderModelPoint::load(DrawingBuffer& buf, const RenderObject::Snapshot& snapshot)
{
    const Atlas::Item& texCoord = _atlas->at(snapshot._type);
    buf.nextVertex();
    buf.writePosition(0, 0, 0);
    buf.writeTexCoordinate(static_cast<uint16_t>((texCoord.left() + texCoord.right()) / 2), static_cast<uint16_t>((texCoord.top() + texCoord.bottom()) / 2));
}

RenderModelPoint::BUILDER::BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _atlas(factory.ensureBuilder<Atlas>(manifest, Constants::Attributes::ATLAS)), _resource_loader_context(resourceLoaderContext)
{
}

sp<RenderModel> RenderModelPoint::BUILDER::build(const sp<Scope>& args)
{
    return sp<RenderModelPoint>::make(_resource_loader_context->renderController(), _atlas->build(args));
}

}
