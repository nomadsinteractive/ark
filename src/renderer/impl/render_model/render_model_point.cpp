#include "renderer/impl/render_model/render_model_point.h"

#include "renderer/base/atlas.h"
#include "renderer/base/drawing_context.h"
#include "renderer/base/model_buffer.h"
#include "renderer/base/render_controller.h"
#include "renderer/base/resource_manager.h"
#include "renderer/base/shader_bindings.h"
#include "renderer/util/index_buffers.h"

namespace ark {

GLModelPoint::GLModelPoint(const sp<Atlas>& atlas)
    : _atlas(atlas)
{
}

sp<ShaderBindings> GLModelPoint::makeShaderBindings(const RenderController& renderController, const sp<PipelineLayout>& pipelineLayout)
{
    const sp<ShaderBindings> bindings = sp<ShaderBindings>::make(RENDER_MODE_POINTS, renderController, pipelineLayout);
    bindings->bindSampler(_atlas->texture());
    return bindings;
}

void GLModelPoint::start(ModelBuffer& buf, RenderController& renderController, const Layer::Snapshot& layerContext)
{
    buf.vertices().setGrowCapacity(layerContext._items.size());
    buf.setIndices(IndexBuffers::snapshot(buf.indexBuffer(), renderController.resourceManager(), Buffer::NAME_POINTS, layerContext._items.size()));
}

void GLModelPoint::load(ModelBuffer& buf, int32_t type, const V& scale)
{
    const Atlas::Item& texCoord = _atlas->at(type);
    buf.nextVertex();
    buf.writePosition(0, 0, 0);
    buf.writeTexCoordinate(static_cast<uint16_t>((texCoord.left() + texCoord.right()) / 2), static_cast<uint16_t>((texCoord.top() + texCoord.bottom()) / 2));
}

GLModelPoint::BUILDER::BUILDER(BeanFactory& factory, const document manifest)
    : _atlas(factory.ensureBuilder<Atlas>(manifest, Constants::Attributes::ATLAS))
{
}

sp<RenderModel> GLModelPoint::BUILDER::build(const sp<Scope>& args)
{
    return sp<GLModelPoint>::make(_atlas->build(args));
}

}