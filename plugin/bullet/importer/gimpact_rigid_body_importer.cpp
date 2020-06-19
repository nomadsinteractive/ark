#include "plugin/bullet/importer/gimpact_rigid_body_importer.h"

#include "core/base/bean_factory.h"
#include "core/dom/dom_document.h"
#include "core/util/documents.h"

#include "renderer/base/buffer.h"
#include "renderer/base/mesh.h"
#include "renderer/base/model.h"
#include "renderer/base/vertex_stream.h"
#include "renderer/inf/model_loader.h"

#include "plugin/bullet/base/collision_shape.h"

#include "BulletCollision/Gimpact/btGImpactShape.h"
#include "BulletCollision/Gimpact/btGImpactCollisionAlgorithm.h"

namespace ark {
namespace plugin {
namespace bullet {


namespace {

class GImpactCollisionShape : public CollisionShape {
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

    std::unordered_map<int32_t, sp<CollisionShape>>& shapes = collider.collisionShapes();
    for(const document& i : manifest->children("model"))
    {
        int32_t type = Documents::ensureAttribute<int32_t>(i, Constants::Attributes::TYPE);
        Model model = _model_loader->load(type);
        shapes[type] = makeCollisionShape(model, Documents::getAttribute<float>(i, "mass", 1.0f));
    }
}

sp<CollisionShape> GImpactRigidBodyImporter::makeCollisionShape(const Model& model, btScalar mass)
{
    btTriangleIndexVertexArray* tiva = new btTriangleIndexVertexArray();
    DCHECK(model.meshes(), "This model has no meshes data");
    for(const Mesh& i : *model.meshes())
    {
        btIndexedMesh indexedMesh;
        indexedMesh.m_numTriangles = static_cast<int32_t>(i.indices()->length() / 3);
        indexedMesh.m_triangleIndexBase = reinterpret_cast<const unsigned char*>(i.indices()->buf());
        indexedMesh.m_triangleIndexStride = 3 * sizeof(element_index_t);
        indexedMesh.m_numVertices = static_cast<int32_t>(i.vertices()->length());
        indexedMesh.m_vertexBase = reinterpret_cast<const unsigned char*>(i.vertices()->buf());
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
}
}
