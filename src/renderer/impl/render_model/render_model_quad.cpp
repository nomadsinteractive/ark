#include "renderer/impl/render_model/render_model_quad.h"

#include "renderer/base/atlas.h"
#include "renderer/base/drawing_context.h"
#include "renderer/base/model_buffer.h"
#include "renderer/base/render_controller.h"
#include "renderer/base/shader_bindings.h"
#include "renderer/util/index_buffers.h"

namespace ark {

GLModelQuad::GLModelQuad(const sp<Atlas>& atlas)
    : _atlas(atlas)
{
}

sp<ShaderBindings> GLModelQuad::makeShaderBindings(const RenderController& renderController, const sp<PipelineLayout>& pipelineLayout)
{
    const sp<ShaderBindings> bindings = sp<ShaderBindings>::make(RENDER_MODE_TRIANGLES, renderController, pipelineLayout);
    bindings->bindSampler(_atlas->texture());
    return bindings;
}

void GLModelQuad::start(ModelBuffer& buf, RenderController& renderController, const Layer::Snapshot& layerContext)
{
    buf.vertices().setGrowCapacity(4 * layerContext._items.size());
    buf.setIndices(IndexBuffers::snapshot(buf.indexBuffer(), renderController.resourceManager(), Buffer::NAME_QUADS, layerContext._items.size()));
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

Metrics GLModelQuad::measure(int32_t type)
{
    const Atlas::Item& texCoord = _atlas->at(type);
    return {{texCoord.width(), texCoord.height()}, {texCoord.width(), texCoord.height()}, {0, 0}};
}

GLModelQuad::BUILDER::BUILDER(BeanFactory& factory, const document manifest)
    : _atlas(factory.ensureBuilder<Atlas>(manifest, Constants::Attributes::ATLAS))
{
}

sp<RenderModel> GLModelQuad::BUILDER::build(const sp<Scope>& args)
{
    return sp<GLModelQuad>::make(_atlas->build(args));
}

}