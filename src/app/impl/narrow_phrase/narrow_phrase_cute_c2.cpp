#include "app/impl/narrow_phrase/narrow_phrase_cute_c2.h"

#include "core/types/box.h"

#include "graphics/base/rect.h"

#include "renderer/base/resource_loader_context.h"

#include "app/inf/collider.h"
#include "app/util/body_def_cute_c2.h"

namespace ark {

NarrowPhraseCuteC2::NarrowPhraseCuteC2(const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
{
    loadShapes(manifest);
    for(const document& i : manifest->children("import"))
    {
        const String& src = Documents::ensureAttribute(i, Constants::Attributes::SRC);
        document content = resourceLoaderContext->documents()->get(src);
        loadShapes(content->ensureChild("bodies"));
    }
}

Box NarrowPhraseCuteC2::makeAABBShape(const Rect& aabb)
{
    return sp<ShapeCuteC2>::make(makeAABBShapeImpl(aabb));
}

Box NarrowPhraseCuteC2::makeBallShape(const V2& position, float radius)
{
    return sp<ShapeCuteC2>::make(makeBallShapeImpl(position, radius));
}

Box NarrowPhraseCuteC2::makeBoxShape(const Rect& bounds)
{
    return sp<ShapeCuteC2>::make(makeBoxShapeImpl(bounds));
}

Box NarrowPhraseCuteC2::makeCapsuleShape(const V2& p1, const V2& p2, float radius)
{
    return sp<ShapeCuteC2>::make(makeCapsuleShapeImpl(p1, p2, radius));
}

Box NarrowPhraseCuteC2::makePolygonShape(const std::vector<V2>& vertices)
{
    return sp<ShapeCuteC2>::make(makePolygonShapeImpl(vertices));
}

NarrowPhrase::Ray NarrowPhraseCuteC2::toRay(const V2& from, const V2& to)
{
    Ray ray;
    toRay(from, to, *ray.data<c2Ray>());
    return ray;
}

bool NarrowPhraseCuteC2::collisionManifold(const BroadPhrase::Candidate& candidateOne, const BroadPhrase::Candidate& candidateOther, CollisionManifold& collisionManifold)
{
    std::vector<sp<ShapeCuteC2>> p1, p2;
    for(const ShapeCuteC2& i : ensureShape(candidateOne, p1))
    {
        const ShapeCuteC2 transformed = i.transform(candidateOne.position, candidateOne.rotation);
        const CollisionFilter& oneFilter = getCollisionFilter(transformed._collision_filter, candidateOne._collision_filter);
        for(const ShapeCuteC2& j : ensureShape(candidateOther, p2))
            if(oneFilter.collisionTest(getCollisionFilter(j._collision_filter, candidateOther._collision_filter)) &&
                    transformed.collideManifold(j.transform(candidateOther.position, candidateOther.rotation), collisionManifold))
                return true;
    }
    return false;
}

bool NarrowPhraseCuteC2::rayCastManifold(const Ray& ray, const BroadPhrase::Candidate& candidate, RayCastManifold& rayCastManifold)
{
    std::vector<sp<ShapeCuteC2>> predefined;
    for(const ShapeCuteC2& i : ensureShape(candidate, predefined))
    {
        const ShapeCuteC2 transformed = i.transform(candidate.position, candidate.rotation);
        if(transformed.rayCastManifold(*ray.data<c2Ray>(), rayCastManifold))
            return true;
    }
    return false;
}

void NarrowPhraseCuteC2::toRay(const V2& from, const V2& to, c2Ray& ray) const
{
    const V2 delta = to - from;
    ray.p = { from.x(), from.y() };
    if(delta.length() > 0.01f)
    {
        const V2 nd = delta.normalize();
        ray.d = { nd.x(), nd.y() };
        ray.t = delta.length();
    }
    else
    {
        ray.d = { 0, 1.0f };
        ray.t = 0.01f;
    }
}

void NarrowPhraseCuteC2::loadShapes(const document& manifest)
{
    for(const document& i : manifest->children("body"))
    {
        sp<BodyDefCuteC2> bodyDef = sp<BodyDefCuteC2>::make(i);
        _body_defs[bodyDef->shapeId()] = std::move(bodyDef);
    }
}

const CollisionFilter& NarrowPhraseCuteC2::getCollisionFilter(const CollisionFilter& oneFilter, const sp<CollisionFilter>& specifiedFilter)
{
    return specifiedFilter ? *specifiedFilter : oneFilter;
}

const std::vector<sp<ShapeCuteC2>>& NarrowPhraseCuteC2::ensureShape(const BroadPhrase::Candidate& candidate, std::vector<sp<ShapeCuteC2>>& predefined) const
{
    if(candidate.shapes.size() > 0)
    {
        predefined = toCuteC2Shapes(candidate.shapes);
        return predefined;
    }
    const auto iter = _body_defs.find(candidate.shape_id);
    DCHECK(iter != _body_defs.end(), "Shape %d not found", candidate.shape_id);
    return iter->second->shapes();
}

std::vector<sp<ShapeCuteC2>> NarrowPhraseCuteC2::toCuteC2Shapes(const std::vector<Box>& boxes) const
{
    std::vector<sp<ShapeCuteC2>> shapes;
    for(const Box& i : boxes)
        shapes.push_back(i.unpack<ShapeCuteC2>());
    return shapes;
}

ShapeCuteC2 NarrowPhraseCuteC2::makeAABBShapeImpl(const Rect& bounds)
{
    ShapeCuteC2 shape;
    shape.t = C2_TYPE_AABB;
    shape.s.aabb.min.x = bounds.left();
    shape.s.aabb.min.y = bounds.top();
    shape.s.aabb.max.x = bounds.right();
    shape.s.aabb.max.y = bounds.bottom();
    return shape;
}

ShapeCuteC2 NarrowPhraseCuteC2::makeBallShapeImpl(const V2& position, float radius)
{
    ShapeCuteC2 shape;
    shape.t = C2_TYPE_CIRCLE;
    shape.s.circle.p.x = position.x();
    shape.s.circle.p.y = position.y();
    shape.s.circle.r = radius;
    return shape;
}

ShapeCuteC2 NarrowPhraseCuteC2::makeBoxShapeImpl(const Rect& bounds)
{
    return makePolygonShapeImpl({V2(bounds.left(), bounds.top()), V2(bounds.left(), bounds.bottom()), V2(bounds.right(), bounds.bottom()), V2(bounds.right(), bounds.top())});
}

ShapeCuteC2 NarrowPhraseCuteC2::makeCapsuleShapeImpl(const V2& p1, const V2& p2, float radius)
{
    ShapeCuteC2 shape;
    shape.t = C2_TYPE_CAPSULE;
    shape.s.capsule.a.x = p1.x();
    shape.s.capsule.a.y = p1.y();
    shape.s.capsule.b.x = p2.x();
    shape.s.capsule.b.y = p2.y();
    shape.s.capsule.r = radius;
    return shape;
}

ShapeCuteC2 NarrowPhraseCuteC2::makePolygonShapeImpl(const std::vector<V2>& vertices)
{
    DCHECK(vertices.size() < C2_MAX_POLYGON_VERTS, "Max polygon vertices exceeded, vertices.size: %d, max size: %d", vertices.size(), C2_MAX_POLYGON_VERTS);

    ShapeCuteC2 shape;
    c2Poly& poly = shape.s.poly;
    shape.t = C2_TYPE_POLY;

    for(size_t i = 0; i < vertices.size(); ++i)
        poly.verts[i] = { vertices.at(i).x(), vertices.at(i).y() };
    poly.count = static_cast<int32_t>(vertices.size());

    c2MakePoly(&poly);
    return shape;
}

NarrowPhraseCuteC2::BUILDER::BUILDER(BeanFactory& /*factory*/, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _manifest(manifest), _resource_loader_context(resourceLoaderContext)
{
}

sp<NarrowPhrase> NarrowPhraseCuteC2::BUILDER::build(const Scope& /*args*/)
{
    return sp<NarrowPhraseCuteC2>::make(_manifest, _resource_loader_context);
}

}
