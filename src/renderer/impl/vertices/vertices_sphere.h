#ifndef ARK_RENDERER_IMPL_VERTICES_VERTICES_SPHERE_H_
#define ARK_RENDERER_IMPL_VERTICES_VERTICES_SPHERE_H_

#include "graphics/base/v3.h"

#include "renderer/base/atlas.h"
#include "renderer/inf/vertices.h"

#include "renderer/impl/model_loader/model_loader_sphere.h"

namespace ark {

class VerticesSphere : public Vertices {
public:
    VerticesSphere(uint32_t sampleCount);
    VerticesSphere(sp<std::vector<ModelLoaderSphere::Vertex>> vertices, const Atlas::Item& item);

    virtual void write(VertexStream& buf, const V3& size) override;

private:
    sp<std::vector<ModelLoaderSphere::Vertex>> _vertices;
    Atlas::Item _item;
};

}


#endif
