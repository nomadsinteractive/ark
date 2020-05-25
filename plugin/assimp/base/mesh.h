#ifndef ARK_PLUGIN_ASSIMP_BASE_MESH_H_
#define ARK_PLUGIN_ASSIMP_BASE_MESH_H_

#include "core/forwarding.h"
#include "core/types/shared_ptr.h"

#include "graphics/base/v3.h"

#include "renderer/forwarding.h"

namespace ark {

class Mesh {
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

public:
    Mesh(array<element_index_t> indices, sp<Array<V3>> vertices, sp<Array<UV>> uvs, sp<Array<V3>> normals, sp<Array<Tangent>> tangents);

    size_t vertexLength() const;

    const array<element_index_t>& indices() const;

    void write(VertexStream& buf, const V3& size) const;

private:
    array<element_index_t> _indices;
    sp<Array<V3>> _vertices;
    sp<Array<UV>> _uvs;
    sp<Array<V3>> _normals;
    sp<Array<Tangent>> _tangents;

};

}

#endif
