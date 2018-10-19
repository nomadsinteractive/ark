#include "renderer/impl/gl_model_loader/gl_model_loader_line_strip.h"

#include "renderer/base/atlas.h"
#include "renderer/base/gl_drawing_context.h"
#include "renderer/base/gl_model_buffer.h"
#include "renderer/base/resource_loader_context.h"
#include "renderer/util/gl_index_buffers.h"

namespace ark {

GLModelLoaderLineStrip::GLModelLoaderLineStrip(const sp<ResourceLoaderContext>& resourceLoaderContext)
    : GLModelLoader(GL_TRIANGLE_STRIP), _ibo(resourceLoaderContext->glResourceManager()->makeGLBuffer(nullptr, GL_ELEMENT_ARRAY_BUFFER, GL_DYNAMIC_DRAW))
{
}

void GLModelLoaderLineStrip::start(GLModelBuffer& buf, GLResourceManager& resourceManager, const Layer::Snapshot& layerContext)
{
    buf.vertices().setGrowCapacity(layerContext._items.size());

    if(layerContext._dirty)
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
        buf.setIndices(_ibo.snapshot(sp<GLBuffer::VectorUploader<glindex_t>>::make(std::move(indices))));
    }
    else
        buf.setIndices(_ibo.snapshot());
}

void GLModelLoaderLineStrip::loadModel(GLModelBuffer& buf, const Atlas& atlas, int32_t type, const V& scale)
{
    if(type)
    {
        const Atlas::Item& texCoord = atlas.at(type);
        buf.nextVertex();
        buf.setPosition(0, 0, 0);
        buf.setTexCoordinate(static_cast<uint16_t>((texCoord.left() + texCoord.right()) / 2), static_cast<uint16_t>((texCoord.top() + texCoord.bottom()) / 2));
    }
    buf.nextModel();
}

GLModelLoaderLineStrip::BUILDER::BUILDER(const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _resource_loader_context(resourceLoaderContext)
{
}

sp<GLModelLoader> GLModelLoaderLineStrip::BUILDER::build(const sp<Scope>& /*args*/)
{
    return sp<GLModelLoaderLineStrip>::make(_resource_loader_context);
}

}
