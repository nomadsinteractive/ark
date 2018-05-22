#include "renderer/impl/gl_model_loader/gl_model_loader_sphere.h"

#include "core/util/math.h"

#include "renderer/base/atlas.h"
#include "renderer/base/gl_model_buffer.h"

namespace ark {

GLModelLoaderSphere::GLModelLoaderSphere(const sp<Atlas>& atlas, uint32_t sampleCount)
    : GLModelLoader(GL_TRIANGLE_STRIP), _atlas(atlas), _sample_count(sampleCount), _vertex_count((sampleCount * 2 + 1) * (sampleCount + 1)),
      _vertices_boiler_plate(sp<DynamicArray<float>>::make(_vertex_count * (3 + 2))),
      _indices_boiler_plate(sp<DynamicArray<glindex_t>>::make(4 * sampleCount * sampleCount + (sampleCount * 2 - 1)))
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
    const uint32_t stride = (sampleCount * 2 + 1) * (sampleCount + 1);
    for(uint32_t i = 0; i < sampleCount * 2; i++)
    {
        glindex_t offset = static_cast<glindex_t>((i * (sampleCount + 1)) + stride * i);
        if(i != 0)
            degenerate(indices, offset);
        (*indices++) = offset;
        for(uint32_t j = 1; j < sampleCount; ++j) {
            (*indices++) = (glindex_t) (offset + j);
            (*indices++) = (glindex_t) (offset + j + sampleCount + 1);
        }
        (*indices++) = (glindex_t) (offset + sampleCount);
    }
}

uint32_t GLModelLoaderSphere::estimateVertexCount(uint32_t renderObjectCount)
{
    return renderObjectCount * _vertex_count;
}

void GLModelLoaderSphere::loadVertices(GLModelBuffer& buf, uint32_t type, const V& size)
{
    float* elements = _vertices_boiler_plate->buf();
    const Atlas::Item& item = _atlas->at(type);
    for(uint32_t i = 0; i < _vertex_count; i++)
    {
        buf.setPosition(elements[0], elements[1], elements[2]);
        uint16_t u = item.left() + static_cast<uint16_t>((item.right() - item.left()) * elements[3]);
        uint16_t v = item.top() + static_cast<uint16_t>((item.bottom() - item.top()) * elements[4]);
        buf.setTexCoordinate(u, v);
        buf.setNormal(V3(elements[0], elements[1], elements[2]));
        buf.nextVertex();
    }
    buf.writeIndices(_indices_boiler_plate->buf(), _indices_boiler_plate->length());
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

}
