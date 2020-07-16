#ifndef ARK_RENDERER_BASE_MODEL_H_
#define ARK_RENDERER_BASE_MODEL_H_

#include "core/forwarding.h"
#include "core/collection/table.h"
#include "core/base/string.h"
#include "core/inf/array.h"
#include "core/types/shared_ptr.h"

#include "graphics/base/metrics.h"
#include "graphics/base/v3.h"

#include "renderer/forwarding.h"
#include "renderer/inf/vertices.h"
#include "renderer/inf/uploader.h"

namespace ark {

class ARK_API Model {
public:
    Model() = default;
    Model(sp<Uploader> indices, sp<Vertices> vertices, const Metrics& metrics = {V3(1.0f), V3(1.0f), V3(0)});
    Model(sp<Array<Mesh>> meshes, const Metrics& metrics = {V3(1.0f), V3(1.0f), V3(0)});
    DEFAULT_COPY_AND_ASSIGN(Model);

    const sp<Uploader>& indices() const;
    const sp<Vertices>& vertices() const;
    const sp<Array<Mesh>>& meshes() const;

    const Metrics& metrics() const;

//[[script::bindings::property]]
    size_t indexLength() const;
//[[script::bindings::property]]
    size_t vertexLength() const;

//[[script::bindings::auto]]
    sp<Animate> makeAnimate(const String& name, const sp<Numeric>& duration) const;

    const Table<String, sp<AnimateMaker>>& animates() const;
    Table<String, sp<AnimateMaker>>& animates();

    void writeToStream(VertexStream& buf, const V3& size) const;

private:
    V3 toScale(const V3& renderObjectSize) const;

private:
    class MeshIndicesUploader : public Uploader {
    public:
        MeshIndicesUploader(sp<ark::Array<Mesh>> meshes);

        virtual void upload(Writable& uploader) override;

    private:
        size_t calcIndicesSize(ark::Array<Mesh>& meshes) const;

    private:
        sp<ark::Array<Mesh>> _meshes;
    };

    class MeshVertices : public Vertices {
    public:
        MeshVertices(sp<Array<Mesh>> meshes);

        virtual void write(VertexStream& buf, const V3& size) override;

    private:
        size_t calcVertexLength(Array<Mesh>& meshes) const;

    private:
        sp<Array<Mesh>> _meshes;
        V3 _size;
    };

private:
    sp<Uploader> _indices;
    sp<Vertices> _vertices;
    sp<Array<Mesh>> _meshes;

    Table<String, sp<AnimateMaker>> _animates;
    Metrics _metrics;
};

}

#endif
