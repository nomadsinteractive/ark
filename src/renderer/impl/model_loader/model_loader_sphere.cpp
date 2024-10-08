#include "renderer/impl/model_loader/model_loader_sphere.h"

#include "core/base/manifest.h"
#include "core/util/documents.h"
#include "core/util/math.h"
#include "core/util/uploader_type.h"

#include "renderer/base/atlas.h"
#include "renderer/base/model.h"
#include "renderer/base/pipeline_descriptor.h"
#include "renderer/base/pipeline_bindings.h"
#include "renderer/base/texture.h"
#include "renderer/impl/vertices/vertices_sphere.h"
#include "renderer/impl/render_command_composer/rcc_draw_elements_instanced.h"
#include "renderer/util/render_util.h"

namespace ark {

namespace {

void buildVertex(float lng, float lat, ModelLoaderSphere::Vertex& vertex)
{
    float r = Math::cos(lat) / 2.0f;
    float px = r * Math::sin(lng);
    float py = Math::sin(lat) / 2.0f;
    float pz = r * Math::cos(lng);
    vertex._position = V3(px, py, pz);
    const V3& normal = vertex._position;
    vertex._tangent = std::abs(normal.x()) < 0.25f ? normal.cross(V3(1.0f, 0.0f, 0.0f)) : normal.cross(V3(0.0f, 1.0f, 0.0f));
}

void buildTexture(float lng, float lat, ModelLoaderSphere::Vertex& vertex)
{
    vertex._u = static_cast<float>(lng / Math::PI / 2);
    vertex._v = static_cast<float>(lat / Math::PI);
}

sp<std::vector<ModelLoaderSphere::Vertex>> makeVertices(uint32_t sampleCount)
{
    std::vector<ModelLoaderSphere::Vertex> vertices((sampleCount * 2 + 1) * (sampleCount + 1));

    ModelLoaderSphere::Vertex* vertex = vertices.data();
    float step = Math::PI / sampleCount;
    for(uint32_t i = 0; i <= sampleCount * 2; ++i) {
        float lng = i * step;
        for(uint32_t j = 0; j <= sampleCount; ++j) {
            float lat = j * step;
            buildVertex(lng, lat - Math::PI_2, *vertex);
            buildTexture(lng, lat, *vertex);
            ++vertex;
        }
    }

    return sp<std::vector<ModelLoaderSphere::Vertex>>::make(std::move(vertices));
}

sp<Uploader> makeIndices(uint32_t sampleCount)
{
    std::vector<element_index_t> indices(2 * 6 * sampleCount * sampleCount);
    element_index_t* buf = indices.data();
    for(uint32_t i = 0; i < sampleCount * 2; i++)
    {
        element_index_t offset = static_cast<element_index_t>(i * (sampleCount + 1));
        (*buf++) = offset;
        (*buf++) = static_cast<element_index_t>(offset + 1 + sampleCount + 1);
        (*buf++) = static_cast<element_index_t>(offset + 1);
        for(uint32_t j = 1; j < sampleCount - 1; ++j) {
            (*buf++) = static_cast<element_index_t>(offset + j);
            (*buf++) = static_cast<element_index_t>(offset + j + sampleCount + 1);
            (*buf++) = static_cast<element_index_t>(offset + j + sampleCount + 2);
            (*buf++) = static_cast<element_index_t>(offset + j);
            (*buf++) = static_cast<element_index_t>(offset + j + sampleCount + 2);
            (*buf++) = static_cast<element_index_t>(offset + j + 1);
        }
        (*buf++) = static_cast<element_index_t>(offset + sampleCount + sampleCount);
        (*buf++) = static_cast<element_index_t>(offset + sampleCount);
        (*buf++) = static_cast<element_index_t>(offset + sampleCount - 1);
    }
    return UploaderType::makeElementIndexInput(std::move(indices));
}

class ModelBundleImporterSphere final : public ModelLoader::Importer {
public:
    virtual Model import(const Manifest& manifest, MaterialBundle& /*materialBundle*/) override {
        uint32_t sampleCount = Documents::ensureAttribute<uint32_t>(manifest.descriptor(), "sample-count");
        const Rect uvBounds = Rect(0, 1.0f, 1.0f, 0);
        sp<std::vector<ModelLoaderSphere::Vertex>> vertices = makeVertices(sampleCount);
        return Model(makeIndices(sampleCount), sp<VerticesSphere>::make(std::move(vertices), uvBounds), nullptr);
    }
};

}

ModelLoaderSphere::ModelLoaderSphere(const sp<Atlas>& atlas, uint32_t sampleCount)
    : ModelLoader(Enum::RENDER_MODE_TRIANGLES, atlas->texture()), _atlas(atlas), _indices(makeIndices(sampleCount)), _vertices(makeVertices(sampleCount))
{
}

sp<RenderCommandComposer> ModelLoaderSphere::makeRenderCommandComposer(const Shader& /*shader*/)
{
    return sp<RCCDrawElementsInstanced>::make(Model(_indices, sp<VerticesSphere>::make(_vertices->size()), nullptr));
}

sp<Model> ModelLoaderSphere::loadModel(int32_t type)
{
    return sp<Model>::make(nullptr, sp<VerticesSphere>::make(_vertices, _atlas->getItemUV(type)), nullptr);
}

ModelLoaderSphere::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _atlas(factory.ensureConcreteClassBuilder<Atlas>(manifest, constants::ATLAS)), _sample_count(Documents::getAttribute<uint32_t>(manifest, "sample-count", 10))
{
}

sp<ModelLoader> ModelLoaderSphere::BUILDER::build(const Scope& args)
{
    return sp<ModelLoaderSphere>::make(_atlas->build(args), _sample_count);
}

ModelLoaderSphere::IMPORTER_BUILDER::IMPORTER_BUILDER()
{
}

sp<ModelLoader::Importer> ModelLoaderSphere::IMPORTER_BUILDER::build(const Scope& args)
{
    return sp<ModelBundleImporterSphere>::make();
}

}
