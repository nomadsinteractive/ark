#include "renderer/util/primitive_model_factory.h"

#include "core/util/uploader_type.h"

#include "graphics/base/metrics.h"

#include "renderer/base/model.h"
#include "renderer/base/render_controller.h"
#include "renderer/base/render_engine.h"
#include "renderer/base/vertex_writer.h"
#include "renderer/inf/vertices.h"

namespace ark {

namespace {

void writeCoordinate(VertexWriter& buf, float u, float v) {
    buf.writeTexCoordinate(static_cast<uint16_t>(u * std::numeric_limits<uint16_t>::max()), static_cast<uint16_t>(v * std::numeric_limits<uint16_t>::max()));
}

V3 toV3(const V4& v4) {
    float w = v4.w() == 0 ? 0.00000001f : v4.w();
    return V3(v4.x() / w, v4.y() / w, v4.z() / w);
}

class VerticesTriangle : public Vertices {
public:
    VerticesTriangle(Optional<Rect> texCoords)
        : Vertices(3), _tex_coords(std::move(texCoords)) {
    }

    virtual void write(VertexWriter& buf, const V3& /*size*/) override {
        const bool hasTexCoords = static_cast<bool>(_tex_coords);
        const Rect texCoords = hasTexCoords ? _tex_coords.value() : Rect();
        buf.next();
        buf.writePosition(V3(-0.5, 0, -0.5));
        if(hasTexCoords)
            writeCoordinate(buf, texCoords.left(), texCoords.bottom());
        buf.writeNormal(V3(0, 1., 0));
        buf.next();
        buf.writePosition(V3(0, 0, 0.5));
        if(hasTexCoords)
            writeCoordinate(buf, (texCoords.left() + texCoords.right()) / 2., texCoords.top());
        buf.writeNormal(V3(0, 1., 0));
        buf.next();
        buf.writePosition(V3(0.5, 0, -0.5));
        if(hasTexCoords)
            writeCoordinate(buf, texCoords.right(), texCoords.bottom());
        buf.writeNormal(V3(0, 1., 0));
    }

private:
    Optional<Rect> _tex_coords;
};

class VerticesPlane : public Vertices {
public:
    VerticesPlane(uint32_t cols, uint32_t rows, Optional<Rect> texCoords, const sp<Mat4>& matrix)
        : Vertices((cols + 1) * (rows + 1)), _cols(cols), _rows(rows), _tex_coords(std::move(texCoords)), _matrix(matrix ? Optional<M4>(matrix->val()) : Optional<M4>()) {
    }

    virtual void write(VertexWriter& buf, const V3& /*size*/) override {
        float gridSize = 1.0f;
        float x = -float(_cols) * gridSize / 2;
        float z = x;
        const bool hasTexCoords = static_cast<bool>(_tex_coords);
        const Rect texCoords = hasTexCoords ? _tex_coords.value() : Rect();
        float ustep = texCoords.width() / _cols;
        float vstep = texCoords.height() / _rows;
        float v = texCoords.top();
        CHECK(hasTexCoords || !buf.hasAttribute(PipelineInput::ATTRIBUTE_NAME_TEX_COORDINATE), "Plane doesn't have a tex coordinate configured, but the shader needs one");
        for(size_t i = 0; i < _rows + 1; ++i) {
            float x0 = x;
            float u = texCoords.left();
            for(size_t j = 0; j < _cols + 1; ++j) {
                buf.next();
                buf.writePosition(_matrix ? toV3(_matrix.value() * V4(V3(x0, 0, z), 1.0f)) : V3(x0, 0, z));
                if(hasTexCoords)
                    writeCoordinate(buf, u, v);
                x0 += gridSize;
                u += ustep;
            }
            z += gridSize;
            v += vstep;
        }
    }

private:
    uint32_t _cols;
    uint32_t _rows;
    Optional<Rect> _tex_coords;
    Optional<M4> _matrix;
};


class UploaderPlane : public Uploader {
public:
    UploaderPlane(uint32_t cols, uint32_t rows)
        : Uploader(cols * rows * 2 * 3 * sizeof(element_index_t)), _cols(cols), _rows(rows) {
    }

    virtual bool update(uint64_t /*timestamp*/) override {
        return false;
    }

    virtual void upload(Writable& buf) override {
        std::vector<element_index_t> indices;
        element_index_t s0 = 0;
        element_index_t s1 = static_cast<element_index_t>(_cols + 1);
        indices.reserve(_cols * _rows * 2 * 3);
        const bool flipWindingOrder = Ark::instance().renderController()->renderEngine()->isLHS();

        for(size_t i = 0; i < _rows; ++i) {
            element_index_t v0 = s0;
            element_index_t v1 = s1;
            for(size_t j = 0; j < _cols; ++j) {
                if(flipWindingOrder) {
                    indices.push_back(v0);
                    indices.push_back(v0 + 1);
                    indices.push_back(v1);
                    indices.push_back(v0 + 1);
                    indices.push_back(v1 + 1);
                    indices.push_back(v1);
                }
                else {
                    indices.push_back(v0);
                    indices.push_back(v1);
                    indices.push_back(v0 + 1);
                    indices.push_back(v0 + 1);
                    indices.push_back(v1);
                    indices.push_back(v1 + 1);
                }
                v0 ++;
                v1 ++;
            }
            s0 = s1;
            s1 += static_cast<element_index_t>(_cols + 1);
        }
        buf.write(indices.data(), indices.size() * sizeof(element_index_t), 0);
    }

private:
    uint32_t _cols;
    uint32_t _rows;
};

}


PrimitiveModelFactory::PrimitiveModelFactory(sp<Mat4> transform)
    : _transform(std::move(transform))
{
}

sp<Model> PrimitiveModelFactory::makeTriangle(Optional<Rect> texCoords)
{
    return sp<Model>::make(UploaderType::makeElementIndexInput(std::vector<element_index_t>{0, 1, 2}), sp<VerticesTriangle>::make(std::move(texCoords)), sp<Metrics>::make(V3(0), V3(1., 1., 0), V3(0)));
}

sp<Model> PrimitiveModelFactory::makePlane(uint32_t cols, uint32_t rows, Optional<Rect> texCoords)
{
    return sp<Model>::make(sp<UploaderPlane>::make(cols, rows), sp<VerticesPlane>::make(cols, rows, std::move(texCoords), _transform), sp<Metrics>::make(V3(0), V3(cols, rows, 0), V3(0)));
}

}
