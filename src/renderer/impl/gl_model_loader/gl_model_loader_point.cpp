#include "renderer/impl/gl_model_loader/gl_model_loader_point.h"

#include "renderer/base/atlas.h"
#include "renderer/base/gl_model_buffer.h"
#include "renderer/base/gl_resource_manager.h"

namespace ark {

GLModelLoaderPoint::GLModelLoaderPoint(const sp<Atlas>& atlas)
    : GLModelLoader(GL_POINTS), _atlas(atlas)
{
}

void GLModelLoaderPoint::load(GLModelBuffer& buf, uint32_t type, const V& scale)
{
    const Atlas::Item& texCoord = _atlas->at(type);
    float halfWidth = scale.x() / 2;
    float halfHeight = scale.y() / 2;

    buf.setPosition(-halfWidth, -halfHeight, 0);
    buf.setTexCoordinate(static_cast<uint16_t>((texCoord.left() + texCoord.right()) / 2), static_cast<uint16_t>((texCoord.top() + texCoord.bottom()) / 2));
    buf.nextVertex();
}

uint32_t GLModelLoaderPoint::estimateVertexCount(uint32_t renderObjectCount)
{
    return renderObjectCount;
}

GLBuffer GLModelLoaderPoint::getPredefinedIndexBuffer(GLResourceManager& glResourceManager, uint32_t renderObjectCount)
{
    return glResourceManager.getGLIndexBuffer(GLResourceManager::BUFFER_NAME_POINTS, renderObjectCount);
}

GLModelLoaderPoint::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _atlas(factory.ensureBuilder<Atlas>(manifest))
{
}

sp<GLModelLoader> GLModelLoaderPoint::BUILDER::build(const sp<Scope>& args)
{
    return sp<GLModelLoaderPoint>::make(_atlas->build(args));
}

}
