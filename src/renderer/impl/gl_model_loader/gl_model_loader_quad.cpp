#include "renderer/impl/gl_model_loader/gl_model_loader_quad.h"

#include "core/util/math.h"

#include "renderer/base/atlas.h"
#include "renderer/base/gl_drawing_context.h"
#include "renderer/base/gl_model_buffer.h"
#include "renderer/base/gl_resource_manager.h"
#include "renderer/base/gl_shader_bindings.h"
#include "renderer/util/gl_index_buffers.h"

namespace ark {

GLModelLoaderQuad::GLModelLoaderQuad()
    : GLModelLoader(GL_TRIANGLES)
{
}

void GLModelLoaderQuad::start(GLModelBuffer& buf, GLResourceManager& resourceManager, const LayerContext::Snapshot& layerContext)
{
    buf.vertices().setGrowCapacity(4 * layerContext._items.size());
    buf.indices() = GLIndexBuffers::makeGLBufferSnapshot(resourceManager, GLBuffer::NAME_QUADS, layerContext._items.size());
}

void GLModelLoaderQuad::loadModel(GLModelBuffer& buf, const Atlas& atlas, uint32_t type, const V& scale)
{
    const Atlas::Item& texCoord = atlas.at(type);
    float width = scale.x() == 0 ? texCoord.width() : scale.x();
    float height = scale.y() == 0 ? texCoord.height() : scale.y();

    buf.nextVertex();
    buf.setPosition(0 - texCoord.pivotX() * width, 0 - texCoord.pivotY() * height, 0);
    buf.setTexCoordinate(texCoord.left(), texCoord.top());

    if(buf.transform().isFrontfaceCCW())
    {
        buf.nextVertex();
        buf.setPosition(0 - texCoord.pivotX() * width, height - texCoord.pivotY() * height, 0);
        buf.setTexCoordinate(texCoord.left(), texCoord.bottom());

        buf.nextVertex();
        buf.setPosition(width - texCoord.pivotX() * width, 0 - texCoord.pivotY() * height, 0);
        buf.setTexCoordinate(texCoord.right(), texCoord.top());
    }
    else
    {
        buf.nextVertex();
        buf.setPosition(width - texCoord.pivotX() * width, 0 - texCoord.pivotY() * height, 0);
        buf.setTexCoordinate(texCoord.right(), texCoord.top());

        buf.nextVertex();
        buf.setPosition(0 - texCoord.pivotX() * width, height - texCoord.pivotY() * height, 0);
        buf.setTexCoordinate(texCoord.left(), texCoord.bottom());
    }
    buf.nextVertex();
    buf.setPosition(width - texCoord.pivotX() * width, height - texCoord.pivotY() * height, 0);
    buf.setTexCoordinate(texCoord.right(), texCoord.bottom());
}

GLModelLoaderQuad::BUILDER::BUILDER()
{
}

sp<GLModelLoader> GLModelLoaderQuad::BUILDER::build(const sp<Scope>& args)
{
    return sp<GLModelLoaderQuad>::make();
}

}
