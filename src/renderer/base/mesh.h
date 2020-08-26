#ifndef ARK_RENDERER_BASE_MESH_H_
#define ARK_RENDERER_BASE_MESH_H_

#include <array>

#include "core/base/api.h"
#include "core/forwarding.h"
#include "core/types/shared_ptr.h"

#include "graphics/base/v3.h"

#include "renderer/forwarding.h"

namespace ark {

class ARK_API Mesh {
public:
    struct ARK_API UV {
        UV() = default;
        UV(uint16_t u, uint16_t v);

        uint16_t _u;
        uint16_t _v;
    };

    struct ARK_API Tangent {
        Tangent() = default;
        Tangent(const V3& tangent, const V3& bitangent);

        V3 _tangent;
        V3 _bitangent;
    };

    struct ARK_API BoneInfo {
        BoneInfo() = default;
        BoneInfo(std::array<float, 4> weights, std::array<int32_t, 4> ids);

        void add(int32_t id, float weight);

        std::array<float, 4> _weights;
        std::array<int32_t, 4> _ids;
    };

public:
    Mesh(array<element_index_t> indices, sp<Array<V3>> vertices, sp<Array<UV>> uvs, sp<Array<V3>> normals, sp<Array<Tangent>> tangents, sp<Array<BoneInfo>> boneInfos, sp<Material> material);

    size_t vertexLength() const;

    const array<element_index_t>& indices() const;
    const array<V3>& vertices() const;
    const sp<Material>& material() const;

    const sp<Integer>& nodeId() const;
    void setNodeId(sp<Integer> nodeId);

    void write(VertexStream& buf) const;

private:
    array<element_index_t> _indices;
    sp<Array<V3>> _vertices;
    sp<Array<UV>> _uvs;
    sp<Array<V3>> _normals;
    sp<Array<Tangent>> _tangents;
    sp<Array<BoneInfo>> _bone_infos;
    sp<Material> _material;

    sp<Integer> _node_id;
};

}

#endif
