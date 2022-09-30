#ifndef ARK_RENDERER_IMPL_VERTICES_VERTICES_SPHERE_H_
#define ARK_RENDERER_IMPL_VERTICES_VERTICES_SPHERE_H_

#include "graphics/base/v3.h"

#include "renderer/inf/vertices.h"

#include "renderer/impl/model_loader/model_loader_sphere.h"

namespace ark {

class VerticesSphere : public Vertices {
public:
    VerticesSphere(size_t length);
    VerticesSphere(sp<std::vector<ModelLoaderSphere::Vertex>> vertices, const Rect& uvBounds);

    virtual void write(VertexWriter& buf, const V3& size) override;

private:
    sp<std::vector<ModelLoaderSphere::Vertex>> _vertices;
    Rect _uv_bounds;
};

}


#endif
