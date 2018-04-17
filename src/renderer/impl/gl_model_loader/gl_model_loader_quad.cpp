#include "renderer/impl/gl_model_loader/gl_model_loader_quad.h"

#include "renderer/base/atlas.h"
#include "renderer/base/gl_model_buffer.h"
#include "renderer/base/gl_resource_manager.h"

namespace ark {

GLModelLoaderQuad::GLModelLoaderQuad(const sp<Atlas>& atlas)
    : GLModelLoader(GL_TRIANGLES), _atlas(atlas)
{
}

void GLModelLoaderQuad::load(GLModelBuffer& buf, uint32_t type, const V& scale)
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

GLModelLoaderQuad::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _atlas(factory.ensureBuilder<Atlas>(manifest))
{
}

sp<GLModelLoader> GLModelLoaderQuad::BUILDER::build(const sp<Scope>& args)
{
    return sp<GLModelLoaderQuad>::make(_atlas->build(args));
}

}
