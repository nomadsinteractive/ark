#ifndef ARK_RENDERER_BASE_MODEL_H_
#define ARK_RENDERER_BASE_MODEL_H_

#include "core/forwarding.h"
#include "core/inf/array.h"
#include "core/types/shared_ptr.h"

#include "graphics/base/v3.h"

namespace ark {

class ARK_API Model {
public:
    struct ARK_API UV {
        UV() = default;
        UV(uint16_t u, uint16_t v);

        uint16_t _u;
        uint16_t _v;
    };

    struct ARK_API Tangents {
        Tangents() = default;
        Tangents(const V3& tangent, const V3& bitangent);

        V3 _tangent;
        V3 _bitangent;
    };

    struct Vertex {
        Vertex(V3* position, UV* uv, V3* normal, Tangents* tangent);

        V3* _position;
        UV* _uv;
        V3* _normal;
        Tangents* _tangent;
    };

public:
    Model(array<element_index_t> indices, array<V3> vertices, array<UV> uvs, array<V3> normals, array<Tangents> tangents);
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(Model);

    const array<element_index_t>& indices() const;

    const array<V3>& vertices() const;
    const array<UV>& uvs() const;
    const array<V3>& normals() const;
    const array<Tangents>& tangents() const;

private:
    array<element_index_t> _indices;

    array<V3> _vertices;
    array<UV> _uvs;
    array<V3> _normals;
    array<Tangents> _tangents;
};

}

#endif
