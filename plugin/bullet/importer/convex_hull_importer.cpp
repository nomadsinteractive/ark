#include "plugin/bullet/importer/convex_hull_importer.h"

#include "core/base/bean_factory.h"
#include "core/dom/dom_document.h"
#include "core/util/documents.h"

#include "renderer/base/buffer.h"
#include "renderer/base/mesh.h"
#include "renderer/base/model.h"
#include "renderer/inf/model_loader.h"

#include "plugin/bullet/base/collision_shape_ref.h"

namespace ark::plugin::bullet {

namespace {

sp<CollisionShapeRef> makeCollisionShape(const Model& model, btScalar mass)
{
    const sp<btConvexHullShape> convexHullShape = sp<btConvexHullShape>::make();

    CHECK(!model.meshes().empty(), "ConvexHullRigidBodyImporter only works with Mesh based models");
    for(const Mesh& i : model.meshes())
        for(const V3& j : i.vertices())
            convexHullShape->addPoint(btVector3(j.x(), j.y(), j.z()), false);

    convexHullShape->recalcLocalAabb();
    convexHullShape->optimizeConvexHull();
    return sp<CollisionShapeRef>::make(convexHullShape, model.content()->size()->val());
}

}
ConvexHullRigidBodyImporter::ConvexHullRigidBodyImporter(sp<ModelLoader> modelLoader)
    : _model_loader(std::move(modelLoader))
{
}

void ConvexHullRigidBodyImporter::import(ColliderBullet& collider, const document& manifest)
{
    HashMap<HashId, sp<CollisionShapeRef>>& shapes = collider.collisionShapes();
    for(const document& i : manifest->children("model"))
    {
        const int32_t type = Documents::ensureAttribute<int32_t>(i, constants::TYPE);
        Model model = _model_loader->loadModel(type);
        shapes[type] = makeCollisionShape(model, Documents::getAttribute<float>(i, "mass", 1.0f));
    }
}

ConvexHullRigidBodyImporter::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _model_loader(factory.ensureBuilder<ModelLoader>(manifest, "model-loader"))
{
}

sp<ColliderBullet::RigidbodyImporter> ConvexHullRigidBodyImporter::BUILDER::build(const Scope& args)
{
    return sp<ConvexHullRigidBodyImporter>::make(_model_loader.build(args));
}

}
