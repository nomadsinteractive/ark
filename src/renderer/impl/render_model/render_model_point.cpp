#include "renderer/impl/render_model/render_model_point.h"

#include "renderer/base/atlas.h"
#include "renderer/base/model_buffer.h"
#include "renderer/base/resource_loader_context.h"
#include "renderer/base/shader_bindings.h"

namespace ark {

GLModelPoint::GLModelPoint(const RenderController& renderController, const sp<Atlas>& atlas)
    : _atlas(atlas), _index_buffer(renderController.getNamedBuffer(NamedBuffer::NAME_POINTS))
{
}

sp<ShaderBindings> GLModelPoint::makeShaderBindings(const RenderController& renderController, const sp<PipelineLayout>& pipelineLayout)
{
    const sp<ShaderBindings> bindings = sp<ShaderBindings>::make(RENDER_MODE_POINTS, renderController, pipelineLayout, renderController.makeVertexBuffer(), _index_buffer->buffer());
    bindings->bindSampler(_atlas->texture());
    return bindings;
}

void GLModelPoint::postSnapshot(RenderController& renderController, Layer::Snapshot& snapshot)
{
    snapshot._index_buffer = _index_buffer->snapshot(renderController.resourceManager(), snapshot._items.size());
}

void GLModelPoint::start(ModelBuffer& buf, RenderController& /*renderController*/, const Layer::Snapshot& snapshot)
{
    buf.vertices().setGrowCapacity(snapshot._items.size());
    buf.setIndices(snapshot._index_buffer);
}

void GLModelPoint::load(ModelBuffer& buf, int32_t type, const V& scale)
{
    const Atlas::Item& texCoord = _atlas->at(type);
    buf.nextVertex();
    buf.writePosition(0, 0, 0);
    buf.writeTexCoordinate(static_cast<uint16_t>((texCoord.left() + texCoord.right()) / 2), static_cast<uint16_t>((texCoord.top() + texCoord.bottom()) / 2));
}

GLModelPoint::BUILDER::BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _atlas(factory.ensureBuilder<Atlas>(manifest, Constants::Attributes::ATLAS)), _resource_loader_context(resourceLoaderContext)
{
}

sp<RenderModel> GLModelPoint::BUILDER::build(const sp<Scope>& args)
{
    return sp<GLModelPoint>::make(_resource_loader_context->renderController(), _atlas->build(args));
}

}
