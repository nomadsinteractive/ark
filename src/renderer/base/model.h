#ifndef ARK_RENDERER_BASE_MODEL_H_
#define ARK_RENDERER_BASE_MODEL_H_

#include "core/forwarding.h"
#include "core/inf/array.h"
#include "core/types/shared_ptr.h"

#include "graphics/base/v3.h"

#include "renderer/forwarding.h"

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

public:
    Model() = default;
    Model(const array<element_index_t>& indices, const sp<Vertices>& vertices, const V3& size = V3(1.0f));
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(Model);

    const array<element_index_t>& indices() const;
    const sp<Vertices>& vertices() const;

    const V3& size() const;

    void writeToStream(VertexStream& buf, const V3& size) const;

private:
    V3 toScale(const V3& renderObjectSize) const;

private:
    array<element_index_t> _indices;

    sp<Vertices> _vertices;

    array<V3> _vert;
    array<UV> _uvs;
    array<V3> _normals;
    array<Tangents> _tangents;

    V3 _size;
};

}

#endif
