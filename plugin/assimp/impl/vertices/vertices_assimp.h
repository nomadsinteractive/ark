#ifndef ARK_PLUGIN_ASSIMP_IMPL_VERTICES_VERTICES_ASSIMP_H_
#define ARK_PLUGIN_ASSIMP_IMPL_VERTICES_VERTICES_ASSIMP_H_

#include "core/forwarding.h"
#include "core/types/shared_ptr.h"

#include "graphics/base/v3.h"

#include "renderer/inf/vertices.h"

#include "assimp/base/mesh.h"

namespace ark {
namespace plugin {
namespace assimp {

class VerticesAssimp : public Vertices {
public:
    VerticesAssimp(sp<Array<Mesh>> meshes);

    virtual void write(VertexStream& buf, const V3& size) override;

private:
    size_t calcVertexLength(Array<Mesh>& meshes) const;

private:
    sp<Array<Mesh>> _meshes;
};

}
}
}

#endif
