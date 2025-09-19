#pragma once

#include "core/base/api.h"
#include "core/inf/writable.h"

#include "graphics/inf/transform.h"
#include "graphics/inf/renderable.h"

#include "renderer/base/mesh.h"
#include "renderer/base/pipeline_layout.h"

namespace ark {

class ARK_API VertexWriter {
public:
    VertexWriter(const PipelineLayout::VertexDescriptor& attributes, bool doTransform, uint32_t size, uint32_t stride, sp<Writable> writer);
    VertexWriter(const PipelineLayout::VertexDescriptor& attributes, bool doTransform, uint32_t size, uint32_t stride, uint8_t* ptr);

    template<typename T> void writeAttribute(const T& value, const Attribute::Usage usage) {
        if(_attribute_offsets._offsets[usage] >= 0)
            write(&value, sizeof(T), _attribute_offsets._offsets[usage]);
    }

    uint32_t stride() const;
    bool hasAttribute(int32_t name) const;

    void writePosition(V3 position);
    void writeNormal(V3 normal);
    void writeTangent(V3 tangent);
    void writeBitangent(V3 bitangent);
    void writeTexCoordinate(uint16_t u, uint16_t v);
    void writeBoneInfo(const Mesh::BoneInfo& boneInfo);

    void write(const void* buf, uint32_t size, uint32_t offset);

    void setRenderable(const Renderable::Snapshot& renderObject);

    void next();

private:
    void writeVaryings();
    void writeArray(ByteArray& array);

private:
    class WriterImpl;

    PipelineLayout::VertexDescriptor _attribute_offsets;
    sp<WriterImpl> _delegate;
    uint32_t _stride;

    bool _do_transform;
    bool _visible;
    Transform* _transform;
    const Transform::Snapshot* _transform_snapshot;
    V3 _translate;

    ByteArray::Borrowed _varying_contents;
};

}
