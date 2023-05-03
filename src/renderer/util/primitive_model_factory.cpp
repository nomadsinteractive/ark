#include "renderer/util/primitive_model_factory.h"

#include "graphics/base/metrics.h"

#include "renderer/base/model.h"
#include "renderer/base/render_controller.h"
#include "renderer/base/render_engine.h"
#include "renderer/base/vertex_writer.h"
#include "renderer/inf/vertices.h"

namespace ark {

namespace {

class VerticesPlane : public Vertices {
public:
    VerticesPlane(size_t subdivision, Optional<Rect> texCoords)
        : Vertices((subdivision + 1) * (subdivision + 1)), _subdivisons(subdivision), _tex_coords(std::move(texCoords)) {
    }

    virtual void write(VertexWriter& buf, const V3& /*size*/) override {
        float gridSize = 1.0f;
        float x = -float(_subdivisons) * gridSize / 2;
        float z = x;
        const bool hasTexCoords = static_cast<bool>(_tex_coords);
        const Rect texCoords = hasTexCoords ? _tex_coords.value() : Rect();
        float ustep = texCoords.width() / _subdivisons;
        float vstep = texCoords.height() / _subdivisons;
        float v = texCoords.top();
        for(size_t i = 0; i < _subdivisons + 1; ++i) {
            z += gridSize;
            float x0 = x;
            float u = texCoords.left();
            for(size_t j = 0; j < _subdivisons + 1; ++j) {
                buf.next();
                buf.writePosition(V3(x0, 0, z));
                if(hasTexCoords)
                    buf.writeTexCoordinate(static_cast<uint16_t>(u * std::numeric_limits<uint16_t>::max()), static_cast<uint16_t>(v * std::numeric_limits<uint16_t>::max()));
                x0 += gridSize;
                u += ustep;
            }
            v += vstep;
        }
    }

private:
    size_t _subdivisons;
    Optional<Rect> _tex_coords;
};


class UploaderPlane : public Uploader {
public:
    UploaderPlane(size_t subdivisions)
        : Uploader(subdivisions * subdivisions * 2 * 3 * sizeof(element_index_t)), _subdivisons(subdivisions) {
    }

    virtual bool update(uint64_t /*timestamp*/) override {
        return false;
    }

    virtual void upload(Writable& buf) override {
        std::vector<element_index_t> indices;
        element_index_t s0 = 0;
        element_index_t s1 = static_cast<element_index_t>(_subdivisons + 1);
        indices.reserve(_subdivisons * _subdivisons * 2 * 3);
        const bool flipWindingOrder = Ark::instance().renderController()->renderEngine()->isLHS();

        for(size_t i = 0; i < _subdivisons; ++i) {
            element_index_t v0 = s0;
            element_index_t v1 = s1;
            for(size_t j = 0; j < _subdivisons; ++j) {
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
            s1 += static_cast<element_index_t>(_subdivisons + 1);
        }
        buf.write(indices.data(), indices.size() * sizeof(element_index_t), 0);
    }

private:
    size_t _subdivisons;
};

}


PrimitiveModelFactory::PrimitiveModelFactory()
{
}

sp<Model> PrimitiveModelFactory::makePlane(uint32_t subdivisons, Optional<Rect> texCoords)
{
    return sp<Model>::make(sp<UploaderPlane>::make(subdivisons), sp<VerticesPlane>::make(subdivisons, std::move(texCoords)), sp<Metrics>::make(V3(0), V3(subdivisons, subdivisons, 0), V3(0)));
}

}
