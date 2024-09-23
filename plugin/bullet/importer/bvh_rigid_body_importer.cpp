#include "plugin/bullet/importer/bvh_rigid_body_importer.h"

#include "core/base/bean_factory.h"
#include "core/dom/dom_document.h"
#include "core/util/documents.h"

#include "renderer/base/buffer.h"
#include "renderer/base/mesh.h"
#include "renderer/base/model.h"
#include "renderer/base/vertex_writer.h"
#include "renderer/inf/model_loader.h"

#include "plugin/bullet/base/collision_shape.h"

namespace ark::plugin::bullet {

namespace {

class BvhCollisionShape : public CollisionShape {
public:
    BvhCollisionShape(btBvhTriangleMeshShape* shape, btTriangleIndexVertexArray* tiva)
        : CollisionShape(shape, 0), _tiva(tiva) {
    }

private:
    op<btTriangleIndexVertexArray> _tiva;
};

}

BvhRigidBodyImporter::BvhRigidBodyImporter(sp<ModelLoader> modelLoader)
    : _model_loader(std::move(modelLoader))
{
}

void BvhRigidBodyImporter::import(ColliderBullet& collider, const document& manifest)
{
    std::unordered_map<int32_t, sp<CollisionShape>>& shapes = collider.collisionShapes();
    for(const document& i : manifest->children("model"))
    {
        int32_t type = Documents::ensureAttribute<int32_t>(i, constants::TYPE);
        Model model = _model_loader->loadModel(type);
        shapes[type] = makeCollisionShape(model);
    }
}

sp<CollisionShape> BvhRigidBodyImporter::makeCollisionShape(const Model& model)
{
    btTriangleIndexVertexArray* tiva = new btTriangleIndexVertexArray();
    CHECK(!model.meshes().empty(), "This model has no meshes data");
    for(const Mesh& i : model.meshes())
    {
        btIndexedMesh indexedMesh;
        indexedMesh.m_numTriangles = static_cast<int32_t>(i.indices().size() / 3);
        indexedMesh.m_triangleIndexBase = reinterpret_cast<const unsigned char*>(i.indices().data());
        indexedMesh.m_triangleIndexStride = 3 * sizeof(element_index_t);
        indexedMesh.m_numVertices = static_cast<int32_t>(i.vertexCount());
        indexedMesh.m_vertexBase = reinterpret_cast<const unsigned char*>(i.vertices().data());
        indexedMesh.m_vertexStride = sizeof(V3);
        tiva->addIndexedMesh(indexedMesh, sizeof(element_index_t) == 4 ? PHY_INTEGER : PHY_SHORT);
    }

    btBvhTriangleMeshShape* bvhShape = new btBvhTriangleMeshShape(tiva, true);
    return sp<BvhCollisionShape>::make(bvhShape, tiva);
}

BvhRigidBodyImporter::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _model_loader(factory.ensureBuilder<ModelLoader>(manifest, "model-loader"))
{
}

sp<ColliderBullet::RigidBodyImporter> BvhRigidBodyImporter::BUILDER::build(const Scope& args)
{
    return sp<RigidBodyImporter>::make<BvhRigidBodyImporter>(_model_loader->build(args));
}

}
