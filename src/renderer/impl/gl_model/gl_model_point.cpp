#include "renderer/impl/gl_model/gl_model_point.h"

#include "core/base/memory_pool.h"
#include "core/inf/array.h"

#include "graphics/base/layer_context.h"
#include "graphics/base/rect.h"
#include "graphics/base/vec2.h"

#include "renderer/base/atlas.h"
#include "renderer/base/gl_attribute.h"
#include "renderer/base/varyings.h"

#include "renderer/base/gl_shader.h"
#include "renderer/base/gl_resource_manager.h"

namespace ark {

GLModelPoint::GLModelPoint(const sp<GLShader>& shader, const sp<Atlas>& texCoords)
    : _atlas(texCoords), _tex_coordinate_offset(shader->getAttribute("TexCoordinate").offset()), _stride(shader->stride())
{
    DCHECK(_tex_coordinate_offset % 2 == 0, "Illegal TexCoordinate offset: %d", _tex_coordinate_offset);
    DCHECK(_tex_coordinate_offset < _stride, "Illegal TexCoordinate offset: %d, stride: %d", _tex_coordinate_offset, _stride);
    _tex_coordinate_offset /= 2;
}

bytearray GLModelPoint::getArrayBuffer(MemoryPool& memoryPool, const LayerContext::Snapshot& renderContext, float x, float y)
{
    uint32_t len = renderContext._items.size() * _stride;

    NOT_NULL(len > 0);

    const bytearray preallocated = memoryPool.allocate(len * 4);

    uint8_t* buf = preallocated->buf();
    for(const RenderObject::Snapshot& i : renderContext._items) {
        const Atlas::Item& texCoord = _atlas->at(i._type);
        const V position = i._position;
        const Transform::Snapshot& transform = i._transform;
        float halfWidth = i._size.x() / 2;
        float halfHeight = i._size.y() / 2;
        float tx = position.x() + x;
        float ty = position.y() + y;
        i._varyings.apply(buf, _stride, 1);
        map(buf, transform, -halfWidth, tx + halfWidth, -halfHeight, ty + halfHeight, (texCoord.left() + texCoord.right()) / 2, (texCoord.top() + texCoord.bottom()) / 2);
    }
    return preallocated;
}

GLBuffer GLModelPoint::getIndexBuffer(GLResourceManager& glResourceManager, const LayerContext::Snapshot& renderContext)
{
    return glResourceManager.getGLIndexBuffer(GLResourceManager::BUFFER_NAME_POINTS, renderContext._items.size());
}

uint32_t GLModelPoint::mode() const
{
    return static_cast<uint32_t>(GL_POINTS);
}

void GLModelPoint::map(uint8_t*& buf, const Transform::Snapshot& snapshot, float x, float tx, float y, float ty, uint16_t texCoordX, uint16_t texCoordY) const
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
