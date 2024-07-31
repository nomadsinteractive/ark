#include "renderer/inf/vertices.h"

#include "renderer/base/shader.h"
#include "renderer/base/vertex_writer.h"

namespace ark {

namespace {

class UploaderVertices : public Uploader {
public:
    UploaderVertices(sp<PipelineInput> pipelineInput, sp<Vertices> vertices, const V3& bounds)
        : Uploader(pipelineInput->getStreamLayout(0).stride() * vertices->length()), _pipeline_input(std::move(pipelineInput)), _vertices(std::move(vertices)), _bounds(bounds) {
    }

    virtual bool update(uint64_t /*timestamp*/) override {
        return false;
    }

public:
    virtual void upload(Writable& writable) override {
        size_t stride = _pipeline_input->getStreamLayout(0).stride();
        PipelineInput::AttributeOffsets attributes(_pipeline_input);
        uint32_t size = static_cast<uint32_t>(_vertices->length() * stride);
        std::vector<uint8_t> buf(size);
        VertexWriter stream(attributes, false, buf.data(), size, stride);
        _vertices->write(stream, _bounds);
        writable.write(buf.data(), size, 0);
    }

private:
    sp<PipelineInput> _pipeline_input;
    sp<Vertices> _vertices;
    V3 _bounds;
};

}


Vertices::Vertices(size_t length)
    : _length(length)
{
}

size_t Vertices::length() const
{
    return _length;
}

sp<Uploader> Vertices::toUploader(sp<Vertices> self, Shader& shader, const V3& bounds)
{
    return sp<UploaderVertices>::make(shader.input(), std::move(self), bounds);
}

}
