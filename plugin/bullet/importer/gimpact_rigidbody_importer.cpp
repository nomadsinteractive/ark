#include "plugin/bullet/importer/gimpact_rigidbody_importer.h"

#include "core/base/bean_factory.h"
#include "core/dom/dom_document.h"
#include "core/types/owned_ptr.h"
#include "core/util/documents.h"

#include "renderer/base/buffer.h"
#include "renderer/base/mesh.h"
#include "renderer/base/model.h"
#include "renderer/base/vertex_writer.h"
#include "renderer/inf/model_loader.h"

#include "plugin/bullet/base/collision_shape_ref.h"

#include "BulletCollision/Gimpact/btGImpactShape.h"
#include "BulletCollision/Gimpact/btGImpactCollisionAlgorithm.h"

namespace ark::plugin::bullet {

namespace {

class GImpactCollisionShape final : public CollisionShapeRef {
public:
    GImpactCollisionShape(sp<btCollisionShape> shape, btTriangleIndexVertexArray* tiva, btScalar mass)
        : CollisionShapeRef(std::move(shape), mass), _tiva(tiva) {
    }

private:
    op<btTriangleIndexVertexArray> _tiva;
};

sp<CollisionShapeRef> makeCollisionShape(const Model& model, btScalar mass)
{
    constexpr PHY_ScalarType indexType = sizeof(element_index_t) == 4 ? PHY_INTEGER : PHY_SHORT;
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
        tiva->addIndexedMesh(indexedMesh, indexType);
    }

    const sp<btGImpactMeshShape> gImpactShape = sp<btGImpactMeshShape>::make(tiva);
    gImpactShape->postUpdate();
    gImpactShape->updateBound();
    return sp<CollisionShapeRef>::make<GImpactCollisionShape>(gImpactShape, tiva, mass);
}

}

GImpactRigidbodyImporter::GImpactRigidbodyImporter(sp<ModelLoader> modelLoader)
    : _model_loader(std::move(modelLoader))
{
}

void GImpactRigidbodyImporter::import(ColliderBullet& collider, const document& manifest)
{
    btCollisionDispatcher * dispatcher = static_cast<btCollisionDispatcher*>(collider.btDynamicWorld()->getDispatcher());
    btGImpactCollisionAlgorithm::registerAlgorithm(dispatcher);

    std::unordered_map<TypeId, sp<CollisionShapeRef>>& shapes = collider.collisionShapes();
    for(const document& i : manifest->children("model"))
    {
        const int32_t type = Documents::ensureAttribute<int32_t>(i, constants::TYPE);
        Model model = _model_loader->loadModel(type);
        shapes[type] = makeCollisionShape(model, Documents::getAttribute<float>(i, "mass", 1.0f));
    }
}

GImpactRigidbodyImporter::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _model_loader(factory.ensureBuilder<ModelLoader>(manifest, "model-loader"))
{
}

sp<ColliderBullet::RigidbodyImporter> GImpactRigidbodyImporter::BUILDER::build(const Scope& args)
{
    return sp<GImpactRigidbodyImporter>::make(_model_loader.build(args));
}

}
