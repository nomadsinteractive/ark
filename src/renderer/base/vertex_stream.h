#ifndef ARK_RENDERER_BASE_VERTEX_BUFFER_H_
#define ARK_RENDERER_BASE_VERTEX_BUFFER_H_

#include "core/base/api.h"

#include "graphics/base/transform.h"
#include "graphics/inf/renderable.h"

#include "renderer/base/buffer.h"
#include "renderer/base/varyings.h"

namespace ark {

class ARK_API VertexStream {
public:
    VertexStream(const Buffer::Attributes& attributes, uint8_t* ptr, size_t size, size_t stride, bool doTransform);

    template<typename T> void write(const T& value, size_t offset = 0) {
        DCHECK(_ptr, "BufferWriter is uninitialized, call next() first");
        DCHECK(sizeof(T) + offset <= _stride, "Stride overflow: sizeof(value) = %d, offset = %d", sizeof(value), offset);
        memcpy(_ptr + offset, &value, sizeof(T));
    }

    template<typename T> void write(const T& value, const int32_t* offsets, int32_t name) {
        if(offsets[name] >= 0)
            write<T>(value, offsets[name]);
    }

    void writePosition(const V3& position);
    void writePosition(float x, float y, float z);
    void writeNormal(float x, float y, float z);
    void writeNormal(const V3& normal);
    void writeTangent(float x, float y, float z);
    void writeTangent(const V3& tangent);
    void writeBitangent(float x, float y, float z);
    void writeBitangent(const V3& bitangent);
    void writeTexCoordinate(uint16_t u, uint16_t v);
    void writeModelId(int32_t modelId);

    void writeModel(const Model& model, const V3& scale);

    void setRenderObject(const Renderable::Snapshot& renderObject);

    void next();

private:
    void applyVaryings();

    void writeArray(ByteArray& array);

private:
    Buffer::Attributes _attributes;

    uint8_t* _ptr;
    uint8_t* _begin;
    uint8_t* _end;

    size_t _stride;

    bool _do_transform;
    bool _visible;
    const Transform::Snapshot* _transform;
    V3 _translate;

    Varyings::Snapshot _varyings;

};

}

#endif
