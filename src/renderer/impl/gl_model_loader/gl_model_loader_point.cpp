#include "renderer/impl/gl_model_loader/gl_model_loader_point.h"

#include "renderer/base/atlas.h"
#include "renderer/base/gl_drawing_context.h"
#include "renderer/base/gl_model_buffer.h"
#include "renderer/base/gl_resource_manager.h"
#include "renderer/base/gl_shader_bindings.h"
#include "renderer/util/gl_index_buffers.h"

namespace ark {

GLModelLoaderPoint::GLModelLoaderPoint()
    : GLModelLoader(GL_POINTS)
{
}

void GLModelLoaderPoint::start(GLModelBuffer& buf, GLResourceManager& resourceManager, const LayerContext::Snapshot& layerContext)
{
    buf.vertices().setGrowCapacity(layerContext._items.size());
    buf.indices() = GLIndexBuffers::makeGLBufferSnapshot(resourceManager, GLBuffer::NAME_POINTS, layerContext._items.size());
}

void GLModelLoaderPoint::loadModel(GLModelBuffer& buf, const Atlas& atlas, uint32_t type, const V& scale)
{
    const Atlas::Item& texCoord = atlas.at(type);
    buf.nextVertex();
    buf.setPosition(0, 0, 0);
    buf.setTexCoordinate(static_cast<uint16_t>((texCoord.left() + texCoord.right()) / 2), static_cast<uint16_t>((texCoord.top() + texCoord.bottom()) / 2));
}

GLModelLoaderPoint::BUILDER::BUILDER()
{
}

sp<GLModelLoader> GLModelLoaderPoint::BUILDER::build(const sp<Scope>& args)
{
    return sp<GLModelLoaderPoint>::make();
}

}
