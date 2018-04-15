#ifndef ARK_RENDERER_BASE_GL_ELEMENTS_BUFFER_H_
#define ARK_RENDERER_BASE_GL_ELEMENTS_BUFFER_H_

#include <vector>

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/base/transform.h"
#include "graphics/base/v3.h"

#include "renderer/forwarding.h"
#include "renderer/base/gl_buffer.h"

namespace ark {

class ARK_API GLElementsBuffer {
public:
    GLElementsBuffer(const sp<MemoryPool>& memoryPool, const uint32_t growCapacity, uint32_t stride, int32_t texCoordinateOffset);
    DEFAULT_COPY_AND_ASSIGN(GLElementsBuffer);

    void setPosition(float x, float y, float z);
    void setTexCoordinate(float u, float v);
    void setTexCoordinate(uint16_t u, uint16_t v);
    void setNormal(const V3& normal);
    void setTangents(const V3& tangents);

    void nextVertex();

    void writeIndices(glindex_t* indices, glindex_t count);
    void updateIndicesBase();

    void setTranslate(const V3& translate);
    void setTransform(const Transform::Snapshot& transform);

    GLBuffer::Snapshot getArrayBufferSnapshot(const GLBuffer& arrayBuffer) const;

private:
    void grow();

private:
    sp<MemoryPool> _memory_pool;
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
    bool _use_transform;
};

}

#endif
