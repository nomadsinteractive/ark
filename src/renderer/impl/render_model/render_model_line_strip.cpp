#include "renderer/impl/render_model/render_model_line_strip.h"

#include "renderer/base/atlas.h"
#include "renderer/base/drawing_context.h"
#include "renderer/base/pipeline_bindings.h"
#include "renderer/base/model_buffer.h"
#include "renderer/base/shader.h"
#include "renderer/base/shader_bindings.h"
#include "renderer/base/resource_loader_context.h"

namespace ark {

GLModelLineStrip::GLModelLineStrip(const sp<ResourceLoaderContext>& resourceLoaderContext, const sp<Atlas>& atlas)
    : _atlas(atlas), _ibo(resourceLoaderContext->renderController()->makeIndexBuffer()), _last_rendered_count(0)
{
}

sp<ShaderBindings> GLModelLineStrip::makeShaderBindings(const Shader& shader)
{
    const sp<ShaderBindings> bindings = shader.makeBindings(RENDER_MODE_TRIANGLE_STRIP);
    bindings->pipelineBindings()->bindSampler(_atlas->texture());
    return bindings;
}

void GLModelLineStrip::postSnapshot(RenderController& /*renderController*/, RenderLayer::Snapshot& snapshot)
{
    if(_last_rendered_count != snapshot._items.size())
    {
        _indices = _ibo.snapshot(sp<Uploader::Vector<glindex_t>>::make(makeIndices(snapshot)));
        _last_rendered_count = snapshot._items.size();
    }
}

std::vector<glindex_t> GLModelLineStrip::makeIndices(const RenderLayer::Snapshot& layerContext)
{
    std::vector<glindex_t> indices;
    glindex_t index = 0;
    bool degenerate = false;
    for(const RenderObject::Snapshot& i : layerContext._items)
    {
        if(i._type == 0)
        {
            if(!indices.empty() && !degenerate)
            {
                indices.push_back(indices.back());
                degenerate = true;
            }
            continue;
        }
        indices.push_back(index);
        if(degenerate)
            indices.push_back(index);
        ++index;
        degenerate = false;
    }
    return indices;
}

void GLModelLineStrip::start(ModelBuffer& buf, const RenderLayer::Snapshot& snapshot)
{
    buf.vertices().setGrowCapacity(snapshot._items.size());
    buf.setIndices(_indices);
}

void GLModelLineStrip::load(ModelBuffer& buf, const RenderObject::Snapshot& snapshot)
{
    if(snapshot._type)
    {
        const Atlas::Item& texCoord = _atlas->at(snapshot._type);
        buf.nextVertex();
        buf.writePosition(0, 0, 0);
        buf.writeTexCoordinate(static_cast<uint16_t>((texCoord.left() + texCoord.right()) / 2), static_cast<uint16_t>((texCoord.top() + texCoord.bottom()) / 2));
    }
    buf.nextModel();
}

GLModelLineStrip::BUILDER::BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _atlas(factory.ensureBuilder<Atlas>(manifest, Constants::Attributes::ATLAS)), _resource_loader_context(resourceLoaderContext)
{
}

sp<RenderModel> GLModelLineStrip::BUILDER::build(const sp<Scope>& args)
{
    return sp<GLModelLineStrip>::make(_resource_loader_context, _atlas->build(args));
}

}
