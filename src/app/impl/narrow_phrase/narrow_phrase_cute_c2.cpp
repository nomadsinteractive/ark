#include "app/impl/narrow_phrase/narrow_phrase_cute_c2.h"

#include "core/types/box.h"

#include "graphics/base/rect.h"

#include "renderer/base/resource_loader_context.h"

#include "app/inf/collider.h"

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
        const ShapeCuteC2 transformed(i, candidateOne.position, candidateOne.rotation);
        for(const ShapeCuteC2& j : ensureShape(candidateOther, p2))
            if(transformed.collideManifold(ShapeCuteC2(j, candidateOther.position, candidateOther.rotation), collisionManifold))
                return true;
    }
    return false;
}

bool NarrowPhraseCuteC2::rayCastManifold(const Ray& ray, const BroadPhrase::Candidate& candidate, RayCastManifold& rayCastManifold)
{
    std::vector<sp<ShapeCuteC2>> predefined;
    for(const ShapeCuteC2& i : ensureShape(candidate, predefined))
    {
        const ShapeCuteC2 transformed(i, candidate.position, candidate.rotation);
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
        int32_t shapeId = Documents::ensureAttribute<int32_t>(i, "name");
        String shapeType = Documents::getAttribute(i, "shape-type");
        std::vector<sp<ShapeCuteC2>> shapes;
        if(shapeType == "capsule")
        {
            const float ax = Documents::ensureAttribute<float>(i, "ax");
            const float ay = Documents::ensureAttribute<float>(i, "ay");
            const float bx = Documents::ensureAttribute<float>(i, "bx");
            const float by = Documents::ensureAttribute<float>(i, "by");
            const float r = Documents::ensureAttribute<float>(i, "r");
            shapes.push_back(sp<ShapeCuteC2>::make(makeCapsuleShapeImpl(V2(ax, ay), V2(bx, by), r)));
        }
        else if(shapeType == "polygon")
        {
            int32_t c = 0;
            ShapeCuteC2 shape;
            for(const document& j : i->children())
            {
                DCHECK(c < C2_MAX_POLYGON_VERTS, "Unable to add more vertex, max count: %d", C2_MAX_POLYGON_VERTS);
                shape.s.poly.verts[c].x = Documents::ensureAttribute<float>(j, "x");
                shape.s.poly.verts[c].y = Documents::ensureAttribute<float>(j, "y");
                c++;
            }
            shape.t = C2_TYPE_POLY;
            shape.s.poly.count = c;
            c2MakePoly(&shape.s.poly);
            shapes.push_back(sp<ShapeCuteC2>::make(shape));
        }
        else
        {
            const document& fixtures = i->ensureChild("fixtures");
            const document& fixture = fixtures->ensureChild("fixture");
            const document& fixture_type = fixture->ensureChild("fixture_type");
            DCHECK(fixture_type->value() == "POLYGON", "Unsupported fixture_type: %s", fixture_type->value().c_str());

            const document& polygons = fixture->ensureChild("polygons");
            for(const document& j : polygons->children("polygon"))
            {
                const std::vector<String> values = j->value().split(',');
                DCHECK(values.size() % 2 == 0, "Illegal vertex points: %s", j->value().c_str());
                ShapeCuteC2 shape;

                for(size_t k = 0; k < values.size(); k += 2)
                {
                    DCHECK(k / 2 < C2_MAX_POLYGON_VERTS, "Unable to add more vertex, max count: %d", C2_MAX_POLYGON_VERTS);
                    shape.s.poly.verts[k / 2].x = Strings::parse<float>(values.at(k));
                    shape.s.poly.verts[k / 2].y = Strings::parse<float>(values.at(k + 1));
                }
                shape.t = C2_TYPE_POLY;
                shape.s.poly.count = static_cast<int32_t>(values.size() / 2);
                c2MakePoly(&shape.s.poly);
                shapes.push_back(sp<ShapeCuteC2>::make(shape));
            }
        }
        _shapes[shapeId] = std::move(shapes);
    }
}

const std::vector<sp<ShapeCuteC2>>& NarrowPhraseCuteC2::ensureShape(const BroadPhrase::Candidate& candidate, std::vector<sp<ShapeCuteC2>>& predefined) const
{
    if(candidate.shapes.size() > 0)
    {
        predefined = toCuteC2Shapes(candidate.shapes);
        return predefined;
    }
    const auto iter = _shapes.find(candidate.shape_id);
    DCHECK(iter != _shapes.end(), "Shape %d not found", candidate.shape_id);
    return iter->second;
}

std::vector<sp<ShapeCuteC2>> NarrowPhraseCuteC2::toCuteC2Shapes(const std::vector<Box>& boxes) const
{
    std::vector<sp<ShapeCuteC2>> shapes;
    for(const Box& i : boxes)
        shapes.push_back(i.unpack<ShapeCuteC2>());
    return shapes;
}

ShapeCuteC2 NarrowPhraseCuteC2::makeAABBShapeImpl(const Rect& bounds) const
{
    ShapeCuteC2 shape;
    shape.t = C2_TYPE_AABB;
    shape.s.aabb.min.x = bounds.left();
    shape.s.aabb.min.y = bounds.top();
    shape.s.aabb.max.x = bounds.right();
    shape.s.aabb.max.y = bounds.bottom();
    return shape;
}

ShapeCuteC2 NarrowPhraseCuteC2::makeBallShapeImpl(const V2& position, float radius) const
{
    ShapeCuteC2 shape;
    shape.t = C2_TYPE_CIRCLE;
    shape.s.circle.p.x = position.x();
    shape.s.circle.p.y = position.y();
    shape.s.circle.r = radius;
    return shape;
}

ShapeCuteC2 NarrowPhraseCuteC2::makeBoxShapeImpl(const Rect& bounds) const
{
    return makePolygonShapeImpl({V2(bounds.left(), bounds.top()), V2(bounds.left(), bounds.bottom()), V2(bounds.right(), bounds.bottom()), V2(bounds.right(), bounds.top())});
}

ShapeCuteC2 NarrowPhraseCuteC2::makeCapsuleShapeImpl(const V2& p1, const V2& p2, float radius) const
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

ShapeCuteC2 NarrowPhraseCuteC2::makePolygonShapeImpl(const std::vector<V2>& vertices) const
{
    DCHECK(vertices.size() < C2_MAX_POLYGON_VERTS, "Max polygon vertices exceeded, vertices.size: %d, max size: %d", vertices.size(), C2_MAX_POLYGON_VERTS);

    ShapeCuteC2 shape;
    c2Poly& poly = shape.s.poly;
    shape.t = C2_TYPE_POLY;

    for(size_t i = 0; i < vertices.size(); ++i)
        poly.verts[i] = { vertices.at(i).x(), vertices.at(i).y() };
    poly.count = vertices.size();

    c2MakePoly(&poly);
    return shape;
}

NarrowPhraseCuteC2::BUILDER::BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _manifest(manifest), _resource_loader_context(resourceLoaderContext)
{
}

sp<NarrowPhrase> NarrowPhraseCuteC2::BUILDER::build(const Scope& args)
{
    return sp<NarrowPhraseCuteC2>::make(_manifest, _resource_loader_context);
}

}
