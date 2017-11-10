#include "renderer/impl/gl_model/gl_model_quad.h"

#include "core/inf/array.h"

#include "graphics/base/layer_context.h"
#include "renderer/base/atlas.h"
#include "graphics/base/filter.h"
#include "graphics/base/rect.h"
#include "graphics/base/size.h"
#include "graphics/base/vec2.h"

#include "renderer/base/gl_shader.h"
#include "renderer/base/gl_resource_manager.h"

namespace ark {

GLModelQuad::GLModelQuad(const sp<GLShader>& shader, const sp<Atlas>& atlas)
    : _atlas(atlas), _tex_coordinate_offset(shader->getAttribute("TexCoordinate").offset()), _stride(shader->stride())
{
    DCHECK(_tex_coordinate_offset % 2 == 0 && _tex_coordinate_offset < _stride, "Illegal TexCoordinate offset: %d, stride: %d", _tex_coordinate_offset, _stride);
    _tex_coordinate_offset /= 2;
}

array<uint8_t> GLModelQuad::getArrayBuffer(GLResourceManager& resourceManager, const LayerContext& renderContext, float x, float y)
{
    uint32_t len = renderContext.items().size() * _stride;

    NOT_NULL(len);

    const array<uint8_t> preallocated = resourceManager.getPreallocatedArray(len * 4);

    uint8_t* buf = preallocated->array();
    for(const LayerContext::Item& i : renderContext.items()) {
        const Atlas::Item& texCoord = _atlas->at(i.renderObject->type());
        const sp<Transform>& transform = i.renderObject->transform();
        const V position = i.renderObject->position()->val();
        float w = i.renderObject->width();
        float h = i.renderObject->height();
        float width = w == 0 ? texCoord.width() : w;
        float height = h == 0 ? texCoord.height() : h;
        const Transform::Snapshot snapshot = transform ? transform->snapshot(texCoord.pivotX(), texCoord.pivotY()) : Transform::Snapshot(texCoord.pivotX(), texCoord.pivotY());
        bool clockwise = snapshot.isFrontfaceCCW();
        float tx = position.x() + x + i.x;
        float ty = position.y() + y + i.y;
        i.renderObject->filter()->setVaryings(buf, _stride, 4);
        if(clockwise ^ g_isOriginBottom)
        {
            map(buf, snapshot, 0.0f, tx, height, ty, texCoord.left(), texCoord.bottom());
            map(buf, snapshot, 0.0f, tx, 0.0f, ty, texCoord.left(), texCoord.top());
            map(buf, snapshot, width, tx, height, ty, texCoord.right(), texCoord.bottom());
            map(buf, snapshot, width, tx, 0.0f, ty, texCoord.right(), texCoord.top());
        }
        else
        {
            map(buf, snapshot, 0.0f, tx, 0.0f, ty, texCoord.left(), texCoord.top());
            map(buf, snapshot, 0.0f, tx, height, ty, texCoord.left(), texCoord.bottom());
            map(buf, snapshot, width, tx, 0.0f, ty, texCoord.right(), texCoord.top());
            map(buf, snapshot, width, tx, height, ty, texCoord.right(), texCoord.bottom());
        }
    }
    return preallocated;
}

GLBuffer GLModelQuad::getIndexBuffer(GLResourceManager& glResourceManager, const LayerContext& renderContext)
{
    return glResourceManager.getGLIndexBuffer(GLResourceManager::BUFFER_NAME_TRANGLES, renderContext.items().size() * 6);
}

uint32_t GLModelQuad::mode() const
{
    return static_cast<uint32_t>(GL_TRIANGLES);
}

void GLModelQuad::map(uint8_t*& buf, const Transform::Snapshot& snapshot, float x, float tx, float y, float ty, uint16_t texCoordX, uint16_t texCoordY) const
{
    GLfloat* fp = reinterpret_cast<GLfloat*>(buf);
    uint16_t* ip = reinterpret_cast<uint16_t*>(buf);
    snapshot.map(x, y, tx, ty, *fp, *(fp + 1));
    fp[2] = 0.0f;
    ip[_tex_coordinate_offset] = texCoordX;
    ip[_tex_coordinate_offset + 1] = texCoordY;
    buf += _stride;
}

}
