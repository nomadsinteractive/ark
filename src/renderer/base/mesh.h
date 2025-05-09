#pragma once

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
    Mesh(uint32_t id, String name, Vector<element_index_t> indices, Vector<V3> vertices, sp<Array<UV>> uvs, sp<Array<V3>> normals, sp<Array<Tangent>> tangents, sp<Array<BoneInfo>> boneInfos, sp<Material> material);
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(Mesh);

    const Node& parent() const;

//  [[script::bindings::property]]
    uint32_t id() const;
//  [[script::bindings::property]]
    const String& name() const;
//  [[script::bindings::property]]
    size_t vertexCount() const;
//  [[script::bindings::property]]
    const sp<Material>& material() const;

//  [[script::bindings::property]]
    const Vector<element_index_t>& indices() const;
//  [[script::bindings::property]]
    const Vector<V3>& vertices() const;
    const sp<Array<V3>>& normals() const;
    const sp<Array<Tangent>>& tangents() const;

    void write(VertexWriter& buf) const;

    std::pair<V3, V3> calculateBoundingAABB() const;

private:
    uint32_t _id;
    String _name;
    Vector<element_index_t> _indices;
    Vector<V3> _vertices;
    sp<Array<UV>> _uvs;
    sp<Array<V3>> _normals;
    sp<Array<Tangent>> _tangents;
    sp<Array<BoneInfo>> _bone_infos;
    sp<Material> _material;

    Node* _parent;
    friend class Node;
};

}

