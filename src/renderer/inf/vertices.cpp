#include "renderer/inf/vertices.h"

#include "renderer/base/shader.h"
#include "renderer/base/vertex_writer.h"

namespace ark {

namespace {

class UploaderVertices : public Uploader {
public:
    UploaderVertices(sp<PipelineLayout> shaderLayout, sp<Vertices> vertices, const V3& bounds)
        : Uploader(shaderLayout->getStreamLayout(0).stride() * vertices->length()), _shader_layout(std::move(shaderLayout)), _vertices(std::move(vertices)), _bounds(bounds) {
    }

    virtual bool update(uint64_t /*timestamp*/) override {
        return false;
    }

public:
    void upload(Writable& writable) override {
        size_t stride = _shader_layout->getStreamLayout(0).stride();
        PipelineLayout::VertexDescriptor attributes(_shader_layout);
        uint32_t size = static_cast<uint32_t>(_vertices->length() * stride);
        std::vector<uint8_t> buf(size);
        VertexWriter stream(attributes, false, buf.data(), size, stride);
        _vertices->write(stream, _bounds);
        writable.write(buf.data(), size, 0);
    }

private:
    sp<PipelineLayout> _shader_layout;
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
    return sp<UploaderVertices>::make(shader.layout(), std::move(self), bounds);
}

}
