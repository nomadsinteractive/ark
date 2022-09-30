#ifndef ARK_RENDERER_IMPL_VERTICES_VERTICES_POINTS_H_
#define ARK_RENDERER_IMPL_VERTICES_VERTICES_POINTS_H_

#include "graphics/base/rect.h"

#include "renderer/base/atlas.h"
#include "renderer/inf/vertices.h"

namespace ark {

class VerticesPoint : public Vertices {
public:
    VerticesPoint();
    VerticesPoint(const Atlas::Item& atlasItem);

    virtual void write(VertexWriter& buf, const V3& size) override;

private:
    Atlas::Item _atlas_item;
};

}


#endif
