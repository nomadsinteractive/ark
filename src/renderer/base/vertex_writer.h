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
    class Writer : public Writable {
    public:
        ~Writer() override = default;

        virtual void nextVertex() = 0;
    };

public:
    VertexWriter(const PipelineLayout::VertexDescriptor& attributes, bool doTransform, sp<Writer> writer);
    VertexWriter(const PipelineLayout::VertexDescriptor& attributes, bool doTransform, uint8_t* ptr, size_t size, size_t stride);

    template<typename T> void write(const T& value, const uint32_t offset = 0) {
        _writer->write(&value, sizeof(T), offset);
    }

    template<typename T> void writeAttribute(const T& value, const int32_t name) {
        if(_attribute_offsets._offsets[name] >= 0)
            write<T>(value, _attribute_offsets._offsets[name]);
    }

    bool hasAttribute(int32_t name) const;

    void writePosition(const V3& position);
    void writeNormal(const V3& normal);
    void writeTangent(const V3& tangent);
    void writeBitangent(const V3& bitangent);
    void writeTexCoordinate(uint16_t u, uint16_t v);
    void writeBoneInfo(const Mesh::BoneInfo& boneInfo);

    void write(const void* buf, uint32_t size, uint32_t offset);

    void setRenderable(const Renderable::Snapshot& renderObject);

    void next();

private:
    class WriterMemory : public Writer {
    public:
        WriterMemory(uint8_t* ptr, uint32_t size, uint32_t stride);

        void nextVertex() override;

        uint32_t write(const void* ptr, uint32_t size, uint32_t offset) override;

    private:
        uint8_t* _ptr;
        uint8_t* _begin;
        uint8_t* _end;

        uint32_t _stride;
    };

private:
    void writeVaryings();
    void writeArray(ByteArray& array);

private:
    PipelineLayout::VertexDescriptor _attribute_offsets;
    sp<Writer> _writer;

    bool _do_transform;
    bool _visible;
    Transform* _transform;
    const Transform::Snapshot* _transform_snapshot;
    V3 _translate;

    ByteArray::Borrowed _varying_contents;

};

}
