#include "renderer/impl/gl_model_loader/gl_model_loader_line_strip.h"

#include "renderer/base/atlas.h"
#include "renderer/base/gl_drawing_context.h"
#include "renderer/base/gl_model_buffer.h"
#include "renderer/base/resource_loader_context.h"
#include "renderer/util/gl_index_buffers.h"

namespace ark {

GLModelLoaderLineStrip::GLModelLoaderLineStrip()
    : GLModelLoader(GL_TRIANGLE_STRIP)
{
}

void GLModelLoaderLineStrip::start(GLModelBuffer& buf, GLResourceManager& resourceManager, const LayerContext::Snapshot& layerContext)
{
    buf.vertices().setGrowCapacity(layerContext._items.size());
    buf.indices() = GLIndexBuffers::makeGLBufferSnapshot(resourceManager, GLBuffer::NAME_POINTS, layerContext._items.size());
}

void GLModelLoaderLineStrip::loadModel(GLModelBuffer& buf, const Atlas& atlas, uint32_t type, const V& scale)
{
    if(type)
    {
        const Atlas::Item& texCoord = atlas.at(type);
        buf.nextVertex();
        buf.setPosition(0, 0, 0);
        buf.setTexCoordinate(static_cast<uint16_t>((texCoord.left() + texCoord.right()) / 2), static_cast<uint16_t>((texCoord.top() + texCoord.bottom()) / 2));
        const glindex_t index = 0;
        buf.writeIndices(&index, 1);
    }
    else
    {
        const glindex_t degenerate[2] = {0, 1};
        buf.writeIndices(degenerate, 2);
    }
    buf.nextModel();
}

GLModelLoaderLineStrip::BUILDER::BUILDER()
{
}

sp<GLModelLoader> GLModelLoaderLineStrip::BUILDER::build(const sp<Scope>& /*args*/)
{
    return sp<GLModelLoaderLineStrip>::make();
}

}
