#ifndef ARK_RENDERER_IMPL_VERTICES_VERTICES_POSITION_UV_H_
#define ARK_RENDERER_IMPL_VERTICES_VERTICES_POSITION_UV_H_

#include "renderer/inf/vertices.h"

namespace ark {

class VerticesPositionUV : public Vertices {
public:

    virtual void write(VertexStream& buf, const V3& size) override;

private:

};

}


#endif
