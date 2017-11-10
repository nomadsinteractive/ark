#include "renderer/impl/gl_model/gl_model_point.h"

#include "core/inf/array.h"

#include "graphics/base/rect.h"
#include "graphics/base/vec2.h"

#include "graphics/base/filter.h"
#include "graphics/base/layer_context.h"
#include "renderer/base/atlas.h"

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

array<uint8_t> GLModelPoint::getArrayBuffer(GLResourceManager& resourceManager, const LayerContext& renderContext, float x, float y)
{
    uint32_t len = renderContext.items().size() * _stride;

    NOT_NULL(len > 0);

    const array<uint8_t> preallocated = resourceManager.getPreallocatedArray(len * 4);

    uint8_t* buf = preallocated->array();
    for(const LayerContext::Item& i : renderContext.items()) {
        const Atlas::Item& texCoord = _atlas->at(i.renderObject->type());
        const sp<Transform>& transform = i.renderObject->transform();
        const V position = i.renderObject->position()->val();
        const Transform::Snapshot snapshot = transform ? transform->snapshot() : Transform::Snapshot(0, 0);
        float tx = position.x() + x + i.x;
        float ty = position.y() + y + i.y;
        i.renderObject->filter()->setVaryings(buf, _stride, 1);
        map(buf, snapshot, 0.0f, tx, 0.0f, ty, (texCoord.left() + texCoord.right()) / 2, (texCoord.top() + texCoord.bottom()) / 2);
    }
    return preallocated;
}

GLBuffer GLModelPoint::getIndexBuffer(GLResourceManager& glResourceManager, const LayerContext& renderContext)
{
    return glResourceManager.getGLIndexBuffer(GLResourceManager::BUFFER_NAME_POINTS, renderContext.items().size());
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
