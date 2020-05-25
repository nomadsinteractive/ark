#include "assimp/impl/vertices/vertices_assimp.h"

#include "core/inf/array.h"

namespace ark {
namespace plugin {
namespace assimp {

VerticesAssimp::VerticesAssimp(sp<Array<Mesh>> meshes)
    : Vertices(calcVertexLength(meshes)), _meshes(std::move(meshes))
{
}

size_t VerticesAssimp::calcVertexLength(Array<Mesh>& meshes) const
{
    size_t length = meshes.length();
    size_t vertexLength = 0;
    Mesh* m = meshes.buf();
    for(size_t i = 0; i < length; ++i)
        vertexLength += m[i].vertexLength();
    return vertexLength;
}

void VerticesAssimp::write(VertexStream& buf, const V3& size)
{
    size_t length = _meshes->length();
    Mesh* m = _meshes->buf();
    for(size_t i = 0; i < length; ++i)
        m[i].write(buf, size);
}

}
}
}
