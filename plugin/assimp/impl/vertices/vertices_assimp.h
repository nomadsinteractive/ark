#ifndef ARK_PLUGIN_ASSIMP_IMPL_VERTICES_VERTICES_ASSIMP_H_
#define ARK_PLUGIN_ASSIMP_IMPL_VERTICES_VERTICES_ASSIMP_H_

#include "core/forwarding.h"
#include "core/types/shared_ptr.h"

#include "graphics/base/v3.h"

#include "renderer/inf/vertices.h"

namespace ark {
namespace plugin {
namespace assimp {

class VerticesAssimp : public Vertices {
public:
    struct UV {
        UV() = default;
        UV(uint16_t u, uint16_t v);

        uint16_t _u;
        uint16_t _v;
    };

    struct Tangent {
        Tangent() = default;
        Tangent(const V3& tangent, const V3& bitangent);

        V3 _tangent;
        V3 _bitangent;
    };

    VerticesAssimp(sp<Array<V3>> vertices, sp<Array<UV>> uvs, sp<Array<V3>> normals, sp<Array<Tangent>> tangents);

    virtual void write(VertexStream& buf, const V3& size) override;

private:
    sp<Array<V3>> _vertices;
    sp<Array<UV>> _uvs;
    sp<Array<V3>> _normals;
    sp<Array<Tangent>> _tangents;
};

}
}
}

#endif
