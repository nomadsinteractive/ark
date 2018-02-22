#include "renderer/impl/gl_model/gl_model_quad.h"

#include "core/inf/array.h"

#include "graphics/base/layer_context.h"
#include "renderer/base/atlas.h"
#include "renderer/base/varyings.h"
#include "graphics/base/rect.h"
#include "graphics/base/size.h"
#include "graphics/base/vec2.h"

#include "renderer/base/gl_attribute.h"
#include "renderer/base/gl_shader.h"
#include "renderer/base/gl_resource_manager.h"

namespace ark {

GLModelQuad::GLModelQuad(const sp<GLShader>& shader, const sp<Atlas>& atlas)
    : _atlas(atlas), _tex_coordinate_offset(shader->getAttribute("TexCoordinate").offset()), _stride(shader->stride())
{
    DCHECK(_tex_coordinate_offset % 2 == 0 && _tex_coordinate_offset < _stride, "Illegal TexCoordinate offset: %d, stride: %d", _tex_coordinate_offset, _stride);
    _tex_coordinate_offset /= 2;
}

array<uint8_t> GLModelQuad::getArrayBuffer(GLResourceManager& resourceManager, const LayerContext::Snapshot& renderContext, float x, float y)
{
    uint32_t len = renderContext._items.size() * _stride;

    NOT_NULL(len);

    const array<uint8_t> preallocated = resourceManager.getPreallocatedArray(len * 4);

    uint8_t* buf = preallocated->array();
    for(const RenderObject::Snapshot& renderObject : renderContext._items) {
        const Atlas::Item& texCoord = _atlas->at(renderObject._type);
        Transform::Snapshot transform = renderObject._transform;
        const V& position = renderObject._position;
        float w = renderObject._size.x();
        float h = renderObject._size.y();
        float width = w == 0 ? texCoord.width() : w;
        float height = h == 0 ? texCoord.height() : h;
        bool clockwise = transform.isFrontfaceCCW();
        float tx = position.x() + x;
        float ty = position.y() + y;
        renderObject._varyings.apply(buf, _stride, 4);
        transform.pivot = V(texCoord.pivotX(), texCoord.pivotY());
        if(clockwise ^ g_isOriginBottom)
        {
            map(buf, transform, 0.0f, tx, height, ty, texCoord.left(), texCoord.bottom());
            map(buf, transform, 0.0f, tx, 0.0f, ty, texCoord.left(), texCoord.top());
            map(buf, transform, width, tx, height, ty, texCoord.right(), texCoord.bottom());
            map(buf, transform, width, tx, 0.0f, ty, texCoord.right(), texCoord.top());
        }
        else
        {
            map(buf, transform, 0.0f, tx, 0.0f, ty, texCoord.left(), texCoord.top());
            map(buf, transform, 0.0f, tx, height, ty, texCoord.left(), texCoord.bottom());
            map(buf, transform, width, tx, 0.0f, ty, texCoord.right(), texCoord.top());
            map(buf, transform, width, tx, height, ty, texCoord.right(), texCoord.bottom());
        }
    }
    return preallocated;
}

GLBuffer GLModelQuad::getIndexBuffer(GLResourceManager& glResourceManager, const LayerContext::Snapshot& renderContext)
{
    return glResourceManager.getGLIndexBuffer(GLResourceManager::BUFFER_NAME_TRANGLES, renderContext._items.size() * 6);
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
