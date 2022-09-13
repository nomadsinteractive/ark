#ifndef ARK_RENDERER_BASE_MODEL_H_
#define ARK_RENDERER_BASE_MODEL_H_

#include "core/forwarding.h"
#include "core/collection/table.h"
#include "core/base/string.h"
#include "core/types/shared_ptr.h"

#include "graphics/base/metrics.h"
#include "graphics/base/v3.h"
#include "graphics/inf/renderable.h"

#include "renderer/forwarding.h"
#include "renderer/inf/vertices.h"
#include "renderer/inf/uploader.h"

namespace ark {

class ARK_API Model {
public:
    Model() = default;
    Model(sp<Uploader> indices, sp<Vertices> vertices, const Metrics& metrics = {V3(1.0f), V3(1.0f), V3(0)});
    Model(std::vector<sp<Material>> materials, std::vector<sp<Mesh>> meshes, const Metrics& metrics = {V3(1.0f), V3(1.0f), V3(0)});
    DEFAULT_COPY_AND_ASSIGN(Model);

    const sp<Uploader>& indices() const;
    const sp<Vertices>& vertices() const;

//[[script::bindings::property]]
    const std::vector<sp<Material>>& materials() const;
//[[script::bindings::property]]
    const std::vector<sp<Mesh>>& meshes() const;

    const Metrics& metrics() const;

//[[script::bindings::property]]
    const V3& bounds() const;
//[[script::bindings::property]]
    const V3& size() const;
//[[script::bindings::property]]
    const V3& origin() const;

//[[script::bindings::property]]
    size_t indexCount() const;
//[[script::bindings::property]]
    size_t vertexCount() const;

//  [[script::bindings::property]]
    const Table<String, sp<Animation>>& animations() const;
    void setAnimations(Table<String, sp<Animation>> animations);

//  [[script::bindings::property]]
    const std::vector<String>& nodeNames() const;
    void setNodeNames(std::vector<String> nodes);

//  [[script::bindings::auto]]
    const sp<Animation>& getAnimation(const String& name) const;

    void writeToStream(VertexStream& buf, const V3& size) const;

    void writeRenderable(VertexStream& buf, const Renderable::Snapshot& renderable) const;

    void dispose();
    bool isDisposed() const;

private:
    V3 toScale(const V3& renderObjectSize) const;

private:
    class MeshIndicesUploader : public Uploader {
    public:
        MeshIndicesUploader(std::vector<sp<Mesh>> meshes);

        virtual void upload(Writable& uploader) override;

    private:
        size_t calcIndicesSize(const std::vector<sp<Mesh>>& meshes) const;

    private:
        std::vector<sp<Mesh>> _meshes;
    };

    class MeshVertices : public Vertices {
    public:
        MeshVertices(std::vector<sp<Mesh>> meshes);

        virtual void write(VertexStream& buf, const V3& size) override;

    private:
        size_t calcVertexLength(const std::vector<sp<Mesh> >& meshes) const;

    private:
        std::vector<sp<Mesh>> _meshes;
        V3 _size;
    };

private:
    sp<Uploader> _indices;
    sp<Vertices> _vertices;
    std::vector<sp<Material>> _materials;
    std::vector<sp<Mesh>> _meshes;
    Table<String, sp<Animation>> _animations;
    std::vector<String> _node_names;
    Metrics _metrics;
};

}

#endif
