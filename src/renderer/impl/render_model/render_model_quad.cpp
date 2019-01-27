#include "renderer/impl/render_model/render_model_quad.h"

#include "renderer/base/atlas.h"
#include "renderer/base/drawing_context.h"
#include "renderer/base/model_buffer.h"
#include "renderer/base/render_controller.h"
#include "renderer/base/resource_loader_context.h"
#include "renderer/base/shader_bindings.h"

namespace ark {

GLModelQuad::GLModelQuad(const RenderController& renderController, const sp<Atlas>& atlas)
    : _atlas(atlas), _index_buffer(renderController.getNamedBuffer(NamedBuffer::NAME_QUADS))
{
}

sp<ShaderBindings> GLModelQuad::makeShaderBindings(const RenderController& renderController, const sp<PipelineLayout>& pipelineLayout)
{
    const sp<ShaderBindings> bindings = sp<ShaderBindings>::make(RENDER_MODE_TRIANGLES, renderController, pipelineLayout, renderController.makeVertexBuffer(), _index_buffer->buffer());
    bindings->bindSampler(_atlas->texture());
    return bindings;
}

void GLModelQuad::start(ModelBuffer& buf, RenderController& /*renderController*/, const Layer::Snapshot& snapshot)
{
    buf.vertices().setGrowCapacity(4 * snapshot._items.size());
    buf.setIndices(snapshot._index_buffer);
}

void GLModelQuad::load(ModelBuffer& buf, int32_t type, const V& scale)
{
    const Atlas::Item& texCoord = _atlas->at(type);
    float width = scale.x() == 0 ? texCoord.width() : scale.x();
    float height = scale.y() == 0 ? texCoord.height() : scale.y();

    buf.nextVertex();
    buf.writePosition(0 - texCoord.pivotX() * width, 0 - texCoord.pivotY() * height, 0);
    buf.writeTexCoordinate(texCoord.left(), texCoord.top());

    if(buf.transform().isFrontfaceCCW())
    {
        buf.nextVertex();
        buf.writePosition(0 - texCoord.pivotX() * width, height - texCoord.pivotY() * height, 0);
        buf.writeTexCoordinate(texCoord.left(), texCoord.bottom());

        buf.nextVertex();
        buf.writePosition(width - texCoord.pivotX() * width, 0 - texCoord.pivotY() * height, 0);
        buf.writeTexCoordinate(texCoord.right(), texCoord.top());
    }
    else
    {
        buf.nextVertex();
        buf.writePosition(width - texCoord.pivotX() * width, 0 - texCoord.pivotY() * height, 0);
        buf.writeTexCoordinate(texCoord.right(), texCoord.top());

        buf.nextVertex();
        buf.writePosition(0 - texCoord.pivotX() * width, height - texCoord.pivotY() * height, 0);
        buf.writeTexCoordinate(texCoord.left(), texCoord.bottom());
    }
    buf.nextVertex();
    buf.writePosition(width - texCoord.pivotX() * width, height - texCoord.pivotY() * height, 0);
    buf.writeTexCoordinate(texCoord.right(), texCoord.bottom());
}

void GLModelQuad::postSnapshot(RenderController& renderController, Layer::Snapshot& snapshot)
{
    snapshot._index_buffer = _index_buffer->snapshot(renderController.resourceManager(), snapshot._items.size());
}

Metrics GLModelQuad::measure(int32_t type)
{
    const Atlas::Item& texCoord = _atlas->at(type);
    return {{texCoord.width(), texCoord.height()}, {texCoord.width(), texCoord.height()}, {0, 0}};
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
