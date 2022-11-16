#ifndef ARK_RENDERER_BASE_MESH_H_
#define ARK_RENDERER_BASE_MESH_H_

#include <array>
#include <vector>

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/base/string.h"
#include "core/types/shared_ptr.h"

#include "graphics/base/v3.h"

#include "renderer/forwarding.h"

namespace ark {

class ARK_API Mesh {
public:
    enum Bone {
        INFO_ARRAY_LENGTH = 4
    };

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

        void add(uint32_t id, float weight);

        std::array<float, INFO_ARRAY_LENGTH> _weights;
        std::array<uint32_t, INFO_ARRAY_LENGTH> _ids;
    };

public:
    Mesh(String name, std::vector<element_index_t> indices, sp<Array<V3>> vertices, sp<Array<UV>> uvs, sp<Array<V3>> normals, sp<Array<Tangent>> tangents, sp<Array<BoneInfo>> boneInfos, sp<Material> material);
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(Mesh);

//  [[script::bindings::property]]
    const String& name() const;
//  [[script::bindings::property]]
    size_t vertexLength() const;
//  [[script::bindings::property]]
    const sp<Material>& material() const;

    const std::vector<element_index_t>& indices() const;
    const array<V3>& vertices() const;
    const sp<Array<V3>>& normals() const;
    const sp<Array<Tangent>>& tangents() const;

//[[script::bindings::property]]
    const sp<Integer>& nodeId() const;
    void setNodeId(sp<Integer> nodeId);

    void write(VertexWriter& buf) const;

private:
    String _name;
    std::vector<element_index_t> _indices;
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
