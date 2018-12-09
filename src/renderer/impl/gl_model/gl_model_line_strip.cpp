#include "renderer/impl/gl_model/gl_model_line_strip.h"

#include "renderer/base/atlas.h"
#include "renderer/base/drawing_context.h"
#include "renderer/base/model_buffer.h"
#include "renderer/base/shader_bindings.h"
#include "renderer/base/resource_loader_context.h"
#include "renderer/opengl/util/gl_index_buffers.h"

namespace ark {

GLModelLineStrip::GLModelLineStrip(const sp<ResourceLoaderContext>& resourceLoaderContext, const sp<Atlas>& atlas)
    : RenderModel(RENDER_MODE_TRIANGLE_STRIP), _atlas(atlas), _ibo(resourceLoaderContext->resourceManager()->makeGLBuffer(nullptr, GL_ELEMENT_ARRAY_BUFFER, GL_DYNAMIC_DRAW))
{
}

void GLModelLineStrip::initialize(ShaderBindings& bindings)
{
    bindings.bindGLTexture(_atlas->texture());
}

std::vector<glindex_t> GLModelLineStrip::makeIndices(const Layer::Snapshot& layerContext)
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

void GLModelLineStrip::start(ModelBuffer& buf, GLResourceManager& /*resourceManager*/, const Layer::Snapshot& layerContext)
{
    buf.vertices().setGrowCapacity(layerContext._items.size());

    if(layerContext._dirty)
    {
        const std::vector<glindex_t> indices = makeIndices(layerContext);
        buf.setIndices(_ibo.snapshot(sp<Buffer::VectorUploader<glindex_t>>::make(std::move(indices))));
    }
    else
        buf.setIndices(_ibo.snapshot());
}

void GLModelLineStrip::load(ModelBuffer& buf, int32_t type, const V& /*scale*/)
{
    if(type)
    {
        const Atlas::Item& texCoord = _atlas->at(type);
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
