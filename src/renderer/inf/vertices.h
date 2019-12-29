#ifndef ARK_RENDERER_INF_VERTICES_H_
#define ARK_RENDERER_INF_VERTICES_H_

#include "core/base/api.h"

#include "graphics/forwarding.h"

#include "renderer/forwarding.h"

namespace ark {

class ARK_API Vertices {
public:
    Vertices(size_t length);
    virtual ~Vertices() = default;

    virtual void write(VertexStream& buf, const V3& size) = 0;

    size_t length() const;

private:
    size_t _length;
};

}

#endif
