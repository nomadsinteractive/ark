#include "renderer/impl/gl_model_loader/gl_model_loader_sphere.h"

#include "core/util/math.h"
#include "core/util/log.h"

#include "renderer/base/atlas.h"
#include "renderer/base/gl_model_buffer.h"
#include "renderer/base/resource_loader_context.h"
#include "renderer/util/gl_index_buffers.h"

namespace ark {

GLModelLoaderSphere::GLModelLoaderSphere(const sp<ResourceLoaderContext>& resourceLoaderContext, uint32_t sampleCount)
    : GLModelLoader(GL_TRIANGLE_STRIP), _sample_count(sampleCount), _vertex_count((sampleCount * 2 + 1) * (sampleCount + 1)),
      _vertices_boiler_plate(sp<DynamicArray<float>>::make(_vertex_count * (3 + 2))),
      _indices_boiler_plate(sp<DynamicArray<glindex_t>>::make(4 * sampleCount * sampleCount + 2 * (sampleCount * 2 - 1))),
      _instance_index(resourceLoaderContext->glResourceManager()->makeGLBuffer(sp<GLBuffer::IndexArrayUploader>::make(_indices_boiler_plate), GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW))
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

    glindex_t* indices = _indices_boiler_plate->buf();
    for(uint32_t i = 0; i < sampleCount * 2; i++)
    {
        glindex_t offset = static_cast<glindex_t>(i * (sampleCount + 1));
        if(i != 0)
            degenerate(indices, offset);
        (*indices++) = offset;
        for(uint32_t j = 1; j < sampleCount; ++j) {
            (*indices++) = static_cast<glindex_t>(offset + j);
            (*indices++) = static_cast<glindex_t>(offset + j + sampleCount + 1);
        }
        (*indices++) = (glindex_t) (offset + sampleCount);
    }
}

void GLModelLoaderSphere::start(GLModelBuffer& buf, GLResourceManager& resourceManager, const LayerContext::Snapshot& layerContext)
{
    buf.vertices().setGrowCapacity(layerContext._items.size() * _vertex_count);
    buf.indices() = _instance_index.snapshot();
}

void GLModelLoaderSphere::loadModel(GLModelBuffer& buf, const Atlas& atlas, uint32_t type, const V& size)
{
    float* elements = _vertices_boiler_plate->buf();
    const Atlas::Item& item = atlas.at(type);
    for(uint32_t i = 0; i < _vertex_count; i++)
    {
        buf.nextVertex();
        buf.setPosition(elements[0] * size.x() + 480, elements[1] * size.x() + 270, elements[2] * size.x());
        uint16_t u = item.left() + static_cast<uint16_t>((item.right() - item.left()) * elements[3]);
        uint16_t v = item.top() + static_cast<uint16_t>((item.bottom() - item.top()) * elements[4]);
        buf.setTexCoordinate(u, v);
        buf.setNormal(V3(elements[0], elements[1], elements[2]));
        elements += 5;
    }
    buf.nextModel();
}

void GLModelLoaderSphere::buildVertex(float*& buffer, float lng, float lat) const
{
    float r = Math::cos(lat);
    float px = r * Math::sin(lng);
    float py = Math::sin(lat);
    float pz = r * Math::cos(lng);
    (*buffer++) = px;
    (*buffer++) = py;
    (*buffer++) = pz;
}

void GLModelLoaderSphere::buildTexture(float*& buffer, float lng, float lat) const
{
    float x = (float) (lng / Math::PI / 2);
    float y = (float) (lat / Math::PI);
    (*buffer++) = x;
    (*buffer++) = y;
}

void GLModelLoaderSphere::degenerate(glindex_t*& buffer, glindex_t index) const
{
    uint16_t d = *(buffer - 1);
    (*buffer++) = d;
    (*buffer++) = index;
}

GLModelLoaderSphere::BUILDER::BUILDER(const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _sample_count(Documents::getAttribute(manifest, "sample-count", 10)), _resource_loader_context(resourceLoaderContext)
{
}

sp<GLModelLoader> GLModelLoaderSphere::BUILDER::build(const sp<Scope>& /*args*/)
{
    return sp<GLModelLoaderSphere>::make(_resource_loader_context, _sample_count);
}

}
