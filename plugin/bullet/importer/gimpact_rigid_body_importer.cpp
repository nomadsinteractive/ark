#include "plugin/bullet/importer/gimpact_rigid_body_importer.h"

#include "core/base/bean_factory.h"
#include "core/dom/dom_document.h"
#include "core/util/documents.h"

#include "renderer/base/buffer.h"
#include "renderer/base/mesh.h"
#include "renderer/base/model.h"
#include "renderer/base/vertex_writer.h"
#include "renderer/inf/model_loader.h"

#include "plugin/bullet/base/collision_shape.h"

#include "BulletCollision/Gimpact/btGImpactShape.h"
#include "BulletCollision/Gimpact/btGImpactCollisionAlgorithm.h"

namespace ark::plugin::bullet {

namespace {

class GImpactCollisionShape final : public CollisionShape {
public:
    GImpactCollisionShape(btGImpactMeshShape* shape, btTriangleIndexVertexArray* tiva, btScalar mass)
        : CollisionShape(shape, mass), _tiva(tiva) {
    }

private:
    op<btTriangleIndexVertexArray> _tiva;
};

}

GImpactRigidBodyImporter::GImpactRigidBodyImporter(sp<ModelLoader> modelLoader)
    : _model_loader(std::move(modelLoader))
{
}

void GImpactRigidBodyImporter::import(ColliderBullet& collider, const document& manifest)
{
    btCollisionDispatcher * dispatcher = static_cast<btCollisionDispatcher*>(collider.btDynamicWorld()->getDispatcher());
    btGImpactCollisionAlgorithm::registerAlgorithm(dispatcher);

    std::unordered_map<TypeId, sp<CollisionShape>>& shapes = collider.collisionShapes();
    for(const document& i : manifest->children("model"))
    {
        const int32_t type = Documents::ensureAttribute<int32_t>(i, constants::TYPE);
        Model model = _model_loader->loadModel(type);
        shapes[type] = makeCollisionShape(model, Documents::getAttribute<float>(i, "mass", 1.0f));
    }
}

sp<CollisionShape> GImpactRigidBodyImporter::makeCollisionShape(const Model& model, btScalar mass)
{
    btTriangleIndexVertexArray* tiva = new btTriangleIndexVertexArray();
    DCHECK(model.meshes().size() > 0, "This model has no meshes data");
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

    btGImpactMeshShape* gImpactShape = new btGImpactMeshShape(tiva);
    gImpactShape->postUpdate();
    gImpactShape->updateBound();
    return sp<GImpactCollisionShape>::make(gImpactShape, tiva, mass);
}

GImpactRigidBodyImporter::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _model_loader(factory.ensureBuilder<ModelLoader>(manifest, "model-loader"))
{
}

sp<ColliderBullet::RigidBodyImporter> GImpactRigidBodyImporter::BUILDER::build(const Scope& args)
{
    return sp<GImpactRigidBodyImporter>::make(_model_loader->build(args));
}

}
