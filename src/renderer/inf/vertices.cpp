#include "renderer/inf/vertices.h"

#include "renderer/base/shader.h"
#include "renderer/base/vertex_writer.h"

namespace ark {

namespace {

class UploaderVertices final : public Uploader {
public:
    UploaderVertices(sp<PipelineLayout> shaderLayout, sp<Vertices> vertices, const V3& bounds)
        : Uploader(shaderLayout->getStreamLayout(0).stride() * vertices->length()), _shader_layout(std::move(shaderLayout)), _vertices(std::move(vertices)), _bounds(bounds) {
    }

    bool update(uint64_t /*timestamp*/) override {
        return false;
    }

public:
    void upload(Writable& writable) override {
        const size_t stride = _shader_layout->getStreamLayout(0).stride();
        const PipelineLayout::VertexDescriptor attributes(_shader_layout);
        const uint32_t size = static_cast<uint32_t>(_vertices->length() * stride);
        Vector<uint8_t> buf(size);
        VertexWriter vertexWriter(attributes, false, buf.data(), size, stride);
        _vertices->write(vertexWriter, _bounds);
        writable.write(buf.data(), size, 0);
    }

private:
    sp<PipelineLayout> _shader_layout;
    sp<Vertices> _vertices;
    V3 _bounds;
};

}


Vertices::Vertices(const size_t length)
    : _length(length)
{
}

size_t Vertices::length() const
{
    return _length;
}

sp<Uploader> Vertices::makeUploader(sp<Vertices> self, const Shader& shader, const V3& bounds)
{
    return sp<Uploader>::make<UploaderVertices>(shader.layout(), std::move(self), bounds);
}

}
