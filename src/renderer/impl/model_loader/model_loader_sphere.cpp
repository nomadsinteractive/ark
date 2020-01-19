#include "renderer/impl/model_loader/model_loader_sphere.h"

#include "core/util/documents.h"
#include "core/util/math.h"

#include "renderer/base/atlas.h"
#include "renderer/base/model.h"
#include "renderer/base/pipeline_bindings.h"
#include "renderer/base/shader_bindings.h"
#include "renderer/base/texture.h"
#include "renderer/impl/vertices/vertices_sphere.h"
#include "renderer/impl/render_command_composer/rcc_instanced_models.h"
#include "renderer/util/element_util.h"

namespace ark {

ModelLoaderSphere::ModelLoaderSphere(const sp<Atlas>& atlas, uint32_t sampleCount)
    : ModelLoader(RenderModel::RENDER_MODE_TRIANGLE_STRIP), _atlas(atlas), _sample_count(sampleCount), _indices(sp<IndexArray::Allocated>::make(4 * sampleCount * sampleCount + 2 * (sampleCount * 2 - 1)))
{
    element_index_t* indices = _indices->buf();
    for(uint32_t i = 0; i < sampleCount * 2; i++)
    {
        element_index_t offset = static_cast<element_index_t>(i * (sampleCount + 1));
        if(i != 0)
            degenerate(indices, offset);
        (*indices++) = offset;
        for(uint32_t j = 1; j < sampleCount; ++j) {
            (*indices++) = static_cast<element_index_t>(offset + j + sampleCount + 1);
            (*indices++) = static_cast<element_index_t>(offset + j);
        }
        (*indices++) = static_cast<element_index_t>(offset + sampleCount);
    }
}

sp<RenderCommandComposer> ModelLoaderSphere::makeRenderCommandComposer()
{
    return sp<RCCInstancedModels>::make(Model(_indices, sp<VerticesSphere>::make(_sample_count)));
}

void ModelLoaderSphere::initialize(ShaderBindings& shaderBindings)
{
    shaderBindings.pipelineBindings()->bindSampler(_atlas->texture());
}

void ModelLoaderSphere::postSnapshot(RenderController& /*renderController*/, RenderLayer::Snapshot& snapshot)
{
    for(const Renderable::Snapshot& i : snapshot._items)
    {
        sp<std::vector<ModelLoaderSphere::Vertex>>& vertices = _vertices[i._type];
        if(!vertices)
            vertices = makeVertices();
    }
}

Model ModelLoaderSphere::load(int32_t type)
{
    return Model(nullptr, sp<VerticesSphere>::make(_vertices.at(type), _atlas->at(type)));
}

sp<std::vector<ModelLoaderSphere::Vertex>> ModelLoaderSphere::makeVertices()
{
    std::vector<Vertex> vertices((_sample_count * 2 + 1) * (_sample_count + 1));

    Vertex* vertex = &vertices[0];
    float step = Math::PI / _sample_count;
    for(uint32_t i = 0; i <= _sample_count * 2; ++i) {
        float lng = i * step;
        for(uint32_t j = 0; j <= _sample_count; ++j) {
            float lat = j * step;
            buildVertex(*vertex, lng, lat - Math::PI_2);
            buildTexture(*vertex, lng, lat);
            ++vertex;
        }
    }

    return sp<std::vector<ModelLoaderSphere::Vertex>>::make(std::move(vertices));
}

void ModelLoaderSphere::buildVertex(Vertex& vertex, float lng, float lat) const
{
    float r = Math::cos(lat) / 2.0f;
    float px = r * Math::sin(lng);
    float py = Math::sin(lat) / 2.0f;
    float pz = r * Math::cos(lng);
    vertex._position = V3(px, py, pz);
    const V3& normal = vertex._position;
    vertex._tangent = std::abs(normal.x()) < 0.25f ? normal.cross(V3(1.0f, 0.0f, 0.0f)) : normal.cross(V3(0.0f, 1.0f, 0.0f));
}

void ModelLoaderSphere::buildTexture(Vertex& vertex, float lng, float lat) const
{
    vertex._u = static_cast<float>(lng / Math::PI / 2);
    vertex._v = static_cast<float>(lat / Math::PI);
}

void ModelLoaderSphere::degenerate(element_index_t*& buffer, element_index_t index) const
{
    uint16_t d = *(buffer - 1);
    (*buffer++) = d;
    (*buffer++) = index;
}

ModelLoaderSphere::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _atlas(factory.ensureConcreteClassBuilder<Atlas>(manifest, Constants::Attributes::ATLAS)), _sample_count(Documents::getAttribute<uint32_t>(manifest, "sample-count", 10))
{
}

sp<ModelLoader> ModelLoaderSphere::BUILDER::build(const Scope& args)
{
    return sp<ModelLoaderSphere>::make(_atlas->build(args), _sample_count);
}

}
