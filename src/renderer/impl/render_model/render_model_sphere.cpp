#include "renderer/impl/render_model/render_model_sphere.h"

#include "core/util/math.h"
#include "core/util/log.h"

#include "renderer/base/atlas.h"
#include "renderer/base/drawing_buffer.h"
#include "renderer/base/pipeline_bindings.h"
#include "renderer/base/shader.h"
#include "renderer/base/shader_bindings.h"
#include "renderer/base/resource_loader_context.h"

namespace ark {

GLModelSphere::GLModelSphere(const sp<ResourceLoaderContext>& resourceLoaderContext, const sp<Atlas>& atlas, uint32_t sampleCount)
    : _atlas(atlas), _vertex_count((sampleCount * 2 + 1) * (sampleCount + 1)),
      _vertices_boiler_plate(sp<FloatArray::Allocated>::make(_vertex_count * (3 + 2))),
      _indices_boiler_plate(sp<IndexArray::Allocated>::make(4 * sampleCount * sampleCount + 2 * (sampleCount * 2 - 1))),
      _instance_index(resourceLoaderContext->renderController()->makeIndexBuffer(Buffer::USAGE_STATIC, sp<IndexArrayUploader>::make(_indices_boiler_plate)))
{
    float* vertices = _vertices_boiler_plate->buf();
    float step = Math::PI / sampleCount;
    for(uint32_t i = 0; i <= sampleCount * 2; ++i) {
        float lng = i * step;
        for(uint32_t j = 0; j <= sampleCount; ++j) {
            float lat = j * step;
            buildVertex(vertices, lng, lat - Math::PI_2);
            buildTexture(vertices, lng, lat);
        }
    }

    element_index_t* indices = _indices_boiler_plate->buf();
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
        (*indices++) = (element_index_t) (offset + sampleCount);
    }
}

sp<ShaderBindings> GLModelSphere::makeShaderBindings(const Shader& shader)
{
    const sp<ShaderBindings> bindings = shader.makeBindings(RENDER_MODE_TRIANGLE_STRIP, shader.renderController()->makeVertexBuffer(), _instance_index);
    bindings->pipelineBindings()->bindSampler(_atlas->texture());
    return bindings;
}

void GLModelSphere::postSnapshot(RenderController& /*renderController*/, RenderLayer::Snapshot& /*snapshot*/)
{
}

void GLModelSphere::start(DrawingBuffer& buf, const RenderLayer::Snapshot& layerContext)
{
    buf.vertices().setGrowCapacity(layerContext._items.size() * _vertex_count);
    buf.setIndices(_instance_index.snapshot());
}

void GLModelSphere::load(DrawingBuffer& buf, const RenderObject::Snapshot& snapshot)
{
    float* elements = _vertices_boiler_plate->buf();
    const Atlas::Item& item = _atlas->at(snapshot._type);
    for(uint32_t i = 0; i < _vertex_count; i++)
    {
        buf.nextVertex();
        buf.writePosition(elements[0], elements[1], elements[2]);
        uint16_t u = item.ux() + static_cast<uint16_t>((item.vx() - item.ux()) * elements[3]);
        uint16_t v = item.uy() + static_cast<uint16_t>((item.vy() - item.uy()) * elements[4]);
        buf.writeTexCoordinate(u, v);

        const V3 normal(elements[0], elements[1], elements[2]);
        const V3 tangent = std::abs(normal.x()) < 0.25f ? normal.cross(V3(1.0f, 0.0f, 0.0f)) : normal.cross(V3(0.0f, 1.0f, 0.0f));
        buf.writeNormal(normal);
        buf.writeTangent(tangent);
        elements += 5;
    }
    buf.nextModel();
}

void GLModelSphere::buildVertex(float*& buffer, float lng, float lat) const
{
    float r = Math::cos(lat) / 2.0f;
    float px = r * Math::sin(lng);
    float py = Math::sin(lat) / 2.0f;
    float pz = r * Math::cos(lng);
    (*buffer++) = px;
    (*buffer++) = py;
    (*buffer++) = pz;
}

void GLModelSphere::buildTexture(float*& buffer, float lng, float lat) const
{
    float x = (float) (lng / Math::PI / 2);
    float y = (float) (lat / Math::PI);
    (*buffer++) = x;
    (*buffer++) = y;
}

void GLModelSphere::degenerate(element_index_t*& buffer, element_index_t index) const
{
    uint16_t d = *(buffer - 1);
    (*buffer++) = d;
    (*buffer++) = index;
}

GLModelSphere::BUILDER::BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _atlas(factory.ensureBuilder<Atlas>(manifest, Constants::Attributes::ATLAS)), _sample_count(Documents::getAttribute(manifest, "sample-count", 10)),
      _resource_loader_context(resourceLoaderContext)
{
}

sp<RenderModel> GLModelSphere::BUILDER::build(const Scope& args)
{
    return sp<GLModelSphere>::make(_resource_loader_context, _atlas->build(args), _sample_count);
}

}
