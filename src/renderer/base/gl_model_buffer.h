#ifndef ARK_RENDERER_BASE_GL_MODEL_BUFFER_H_
#define ARK_RENDERER_BASE_GL_MODEL_BUFFER_H_

#include <vector>

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/base/render_object.h"
#include "graphics/base/transform.h"
#include "graphics/base/v3.h"

#include "renderer/forwarding.h"
#include "renderer/base/gl_buffer.h"
#include "renderer/base/varyings.h"

namespace ark {

class ARK_API GLModelBuffer {
public:
    GLModelBuffer(const sp<ResourceLoaderContext>& resourceLoaderContext, const uint32_t growCapacity, uint32_t stride, int32_t texCoordinateOffset);
    DEFAULT_COPY_AND_ASSIGN(GLModelBuffer);

    void setPosition(float x, float y, float z);
    void setTexCoordinate(uint16_t u, uint16_t v);
    void setNormal(const V3& normal);
    void setTangents(const V3& tangents);

    void nextVertex();

    void writeIndices(glindex_t* indices, glindex_t count);
    void updateIndicesBase();

    void setTranslate(const V3& translate);
    void setRenderObject(const RenderObject::Snapshot& renderObject);

    GLBuffer::Snapshot getArrayBufferSnapshot(const GLBuffer& arrayBuffer) const;

private:
    void applyVaryings();
    void grow();

private:
    sp<ResourceLoaderContext> _resource_loader_context;
    uint32_t _grow_capacity;
    std::vector<bytearray> _array_buffers;
    uint8_t* _array_buffer_ptr;
    uint8_t* _array_buffer_boundary;
    std::vector<glindex_t> _indices;

    uint32_t _stride;

    int32_t _tex_coordinate_offset;
    int32_t _normal_offset;
    int32_t _tangents_offset;

    glindex_t _indice_base;
    uint32_t _size;

    Transform::Snapshot _transform;
    V3 _translate;

    Varyings::Snapshot _varyings;
};

}

#endif
