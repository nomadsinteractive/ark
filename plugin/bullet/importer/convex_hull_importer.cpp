#include "plugin/bullet/importer/convex_hull_importer.h"

#include "core/base/bean_factory.h"
#include "core/dom/dom_document.h"
#include "core/util/documents.h"

#include "renderer/base/buffer.h"
#include "renderer/base/model.h"
#include "renderer/base/vertex_stream.h"
#include "renderer/inf/model_loader.h"

#include "plugin/bullet/base/collision_shape.h"

namespace ark {
namespace plugin {
namespace bullet {


namespace {

class ConvexHullVertexWriter : public VertexStream::Writer {
public:
    ConvexHullVertexWriter(btConvexHullShape* convexHullShape)
        : _convex_hull_shape(convexHullShape) {
    }

    virtual void next() override {
    }

    virtual void writePosition(const V3& position) override {
        _convex_hull_shape->addPoint(btVector3(position.x(), position.y(), position.z()));
    }

    virtual void write(const void* ptr, uint32_t size, uint32_t offset) override {
    }

private:
    btConvexHullShape* _convex_hull_shape;
};

}

ConvexHullRigidBodyImporter::ConvexHullRigidBodyImporter(sp<ModelLoader> modelLoader)
    : _model_loader(std::move(modelLoader))
{
}

void ConvexHullRigidBodyImporter::import(ColliderBullet& collider, const document& manifest)
{
    std::unordered_map<int32_t, sp<CollisionShape>>& shapes = collider.collisionShapes();
    for(const document& i : manifest->children("model"))
    {
        int32_t type = Documents::ensureAttribute<int32_t>(i, Constants::Attributes::TYPE);
        Model model = _model_loader->load(type);
        shapes[type] = makeCollisionShape(model, Documents::getAttribute<float>(i, "mass", 1.0f));
    }
}

sp<CollisionShape> ConvexHullRigidBodyImporter::makeCollisionShape(const Model& model, btScalar mass)
{
    btConvexHullShape* convexHullShape = new btConvexHullShape();
    VertexStream vs(Buffer::Attributes(), false, sp<ConvexHullVertexWriter>::make(convexHullShape));
    model.writeToStream(vs, V3(1.0f));
    convexHullShape->recalcLocalAabb();
    convexHullShape->optimizeConvexHull();
    return sp<CollisionShape>::make(convexHullShape, mass);
}

ConvexHullRigidBodyImporter::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _model_loader(factory.ensureBuilder<ModelLoader>(manifest, "model-loader"))
{
}

sp<ColliderBullet::RigidBodyImporter> ConvexHullRigidBodyImporter::BUILDER::build(const Scope& args)
{
    return sp<ConvexHullRigidBodyImporter>::make(_model_loader->build(args));
}

}
}
}
