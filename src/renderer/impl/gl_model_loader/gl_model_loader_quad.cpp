#include "renderer/impl/gl_model_loader/gl_model_loader_quad.h"

#include "renderer/base/atlas.h"
#include "renderer/base/gl_elements_buffer.h"
#include "renderer/base/gl_resource_manager.h"

namespace ark {

GLModelLoaderQuad::GLModelLoaderQuad(const sp<Atlas>& atlas)
    : _atlas(atlas)
{
}

void GLModelLoaderQuad::load(GLElementsBuffer& buf, uint32_t type, const V& scale)
{
    const Atlas::Item& texCoord = _atlas->at(type);
    float width = scale.x() == 0 ? texCoord.width() : scale.x();
    float height = scale.y() == 0 ? texCoord.height() : scale.y();

    buf.setPosition(0 - texCoord.pivotX(), 0 - texCoord.pivotY(), 0);
    buf.setTexCoordinate(texCoord.left(), texCoord.top());
    buf.nextVertex();

    buf.setPosition(0 - texCoord.pivotX(), height - texCoord.pivotY(), 0);
    buf.setTexCoordinate(texCoord.left(), texCoord.bottom());
    buf.nextVertex();

    buf.setPosition(width - texCoord.pivotX(), 0 - texCoord.pivotY(), 0);
    buf.setTexCoordinate(texCoord.right(), texCoord.top());
    buf.nextVertex();

    buf.setPosition(width - texCoord.pivotX(), height - texCoord.pivotY(), 0);
    buf.setTexCoordinate(texCoord.right(), texCoord.bottom());
    buf.nextVertex();
}

uint32_t GLModelLoaderQuad::estimateVertexCount(uint32_t renderObjectCount)
{
    return 4 * renderObjectCount;
}

GLBuffer GLModelLoaderQuad::getPredefinedIndexBuffer(GLResourceManager& glResourceManager, uint32_t renderObjectCount)
{
    return glResourceManager.getGLIndexBuffer(GLResourceManager::BUFFER_NAME_TRANGLES, renderObjectCount * 6);
}

}
