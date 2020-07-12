#ifndef ARK_RENDERER_BASE_VERTEX_BUFFER_H_
#define ARK_RENDERER_BASE_VERTEX_BUFFER_H_

#include "core/base/api.h"
#include "core/inf/writable.h"

#include "graphics/base/transform.h"
#include "graphics/inf/renderable.h"

#include "renderer/base/pipeline_input.h"
#include "renderer/base/varyings.h"

namespace ark {

class ARK_API VertexStream {
public:
    class Writer : public Writable {
    public:
        virtual ~Writer() = default;

        virtual void next() = 0;
        virtual void writePosition(const V3& position) = 0;
    };

public:
    VertexStream(const PipelineInput::Attributes& attributes, bool doTransform, uint8_t* ptr, size_t size, size_t stride);
    VertexStream(const PipelineInput::Attributes& attributes, bool doTransform, sp<Writer> writer);

    template<typename T> void write(const T& value, size_t offset = 0) {
        _writer->write(&value, sizeof(T), offset);
    }

    template<typename T> void write(const T& value, const int32_t* offsets, int32_t name) {
        if(offsets[name] >= 0)
            write<T>(value, offsets[name]);
    }

    void writePosition(const V3& position, uint32_t vertexId);
    void writeNormal(const V3& normal);
    void writeTangent(const V3& tangent);
    void writeBitangent(const V3& bitangent);
    void writeTexCoordinate(uint16_t u, uint16_t v);

    void setRenderObject(const Renderable::Snapshot& renderObject);

    void next();

private:
    class WriterMemory : public Writer {
    public:
        WriterMemory(uint8_t* ptr, uint32_t size, uint32_t stride);

        virtual void next() override;
        virtual void writePosition(const V3& position) override;
        virtual uint32_t write(const void* ptr, uint32_t size, uint32_t offset) override;

    private:
        uint8_t* _ptr;
        uint8_t* _begin;
        uint8_t* _end;

        uint32_t _stride;
    };

private:
    void applyVaryings();

    void writeArray(ByteArray& array);

private:
    PipelineInput::Attributes _attributes;
    sp<Writer> _writer;

    bool _do_transform;
    bool _visible;
    const Transform::Snapshot* _transform;
    V3 _translate;

    Varyings::Snapshot _varyings;

};

}

#endif
