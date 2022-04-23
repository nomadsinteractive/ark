#include "app/impl/narrow_phrase/narrow_phrase_cute_c2.h"

#include "graphics/base/rect.h"

#include "renderer/base/resource_loader_context.h"

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

void NarrowPhraseCuteC2::addAABBShape(int32_t id, const Rect& aabb)
{
    ShapeCuteC2 shape;
    shape.t = C2_TYPE_AABB;
    shape.s.aabb.min.x = aabb.left();
    shape.s.aabb.min.y = aabb.top();
    shape.s.aabb.max.x = aabb.right();
    shape.s.aabb.max.y = aabb.bottom();
    _shapes[id].push_back(shape);
}

void NarrowPhraseCuteC2::addBallShape(int32_t id, const V2& position, float radius)
{
    ShapeCuteC2 shape;
    shape.t = C2_TYPE_CIRCLE;
    shape.s.circle.p.x = position.x();
    shape.s.circle.p.y = position.y();
    shape.s.circle.r = radius;
    _shapes[id].push_back(shape);
}

void NarrowPhraseCuteC2::addBoxShape(int32_t id, const Rect& bounds)
{
    c2Poly box;
    box.count = 4;
    box.verts[0] = {bounds.left(), bounds.top()};
    box.verts[1] = {bounds.left(), bounds.bottom()};
    box.verts[2] = {bounds.right(), bounds.bottom()};
    box.verts[3] = {bounds.right(), bounds.top()};
    addPolygonShape(id, box);
}

void NarrowPhraseCuteC2::addCapsuleShape(int32_t id, const V2& p1, const V2& p2, float radius)
{
    ShapeCuteC2 shape;
    shape.t = C2_TYPE_CAPSULE;
    shape.s.capsule.a.x = p1.x();
    shape.s.capsule.a.y = p1.y();
    shape.s.capsule.b.x = p2.x();
    shape.s.capsule.b.y = p2.y();
    shape.s.capsule.r = radius;
    _shapes[id].push_back(shape);
}

void NarrowPhraseCuteC2::addPolygonShape(int32_t id, const std::vector<V2>& vertices)
{
    c2Poly poly;
    poly.count = static_cast<int32_t>(vertices.size());
    for(size_t i = 0; i < vertices.size(); ++i)
    {
        const V2& p = vertices.at(i);
        poly.verts[i] = {p.x(), p.y()};
    }
    addPolygonShape(id, poly);
}

NarrowPhrase::Ray NarrowPhraseCuteC2::toRay(const V2& from, const V2& to)
{
    Ray ray;
    toRay(from, to, *ray.data<c2Ray>());
    return ray;
}

bool NarrowPhraseCuteC2::collisionManifold(const BroadPhrase::Candidate& candidateOne, const BroadPhrase::Candidate& candidateOther, CollisionManifold& collisionManifold)
{
    const auto iter1 = _shapes.find(candidateOne.shape_id);
    const auto iter2 = _shapes.find(candidateOther.shape_id);
    DCHECK(iter1 != _shapes.end(), "Shape %d not found", candidateOne.shape_id);
    DCHECK(iter2 != _shapes.end(), "Shape %d not found", candidateOther.shape_id);
    for(const ShapeCuteC2& i : iter1->second)
    {
        const ShapeCuteC2 transformed(i, candidateOne.position, candidateOne.rotation);
        for(const ShapeCuteC2& j : iter2->second)
            if(transformed.collideManifold(ShapeCuteC2(j, candidateOther.position, candidateOther.rotation), collisionManifold))
                return true;
    }
    return false;
}

bool NarrowPhraseCuteC2::rayCastManifold(const Ray& ray, const BroadPhrase::Candidate& candidate, RayCastManifold& rayCastManifold)
{
    const auto iter = _shapes.find(candidate.shape_id);
    DCHECK(iter != _shapes.end(), "Shape %d not found", candidate.shape_id);
    for(const ShapeCuteC2& i : iter->second)
    {
        const ShapeCuteC2 transformed(i, candidate.position, candidate.rotation);
        if(transformed.rayCastManifold(*ray.data<c2Ray>(), rayCastManifold))
            return true;
    }
    return false;
}

void NarrowPhraseCuteC2::addPolygonShape(int32_t id, const c2Poly& poly)
{
    ShapeCuteC2 shape;
    shape.t = C2_TYPE_POLY;
    shape.s.poly = poly;
    c2MakePoly(&shape.s.poly);
    _shapes[id].push_back(shape);
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
        ShapeCuteC2 shape;
        std::vector<ShapeCuteC2> shapes;
        if(shapeType == "capsule")
        {
            shape.t = C2_TYPE_CAPSULE;
            shape.s.capsule.a.x = Documents::ensureAttribute<float>(i, "ax");
            shape.s.capsule.a.y = Documents::ensureAttribute<float>(i, "ay");
            shape.s.capsule.b.x = Documents::ensureAttribute<float>(i, "bx");
            shape.s.capsule.b.y = Documents::ensureAttribute<float>(i, "by");
            shape.s.capsule.r = Documents::ensureAttribute<float>(i, "r");
            shapes.push_back(shape);
        }
        else if(shapeType == "polygon")
        {
            int32_t c = 0;
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
            shapes.push_back(shape);
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

                for(size_t k = 0; k < values.size(); k += 2)
                {
                    DCHECK(k / 2 < C2_MAX_POLYGON_VERTS, "Unable to add more vertex, max count: %d", C2_MAX_POLYGON_VERTS);
                    shape.s.poly.verts[k / 2].x = Strings::parse<float>(values.at(k));
                    shape.s.poly.verts[k / 2].y = Strings::parse<float>(values.at(k + 1));
                }
                shape.t = C2_TYPE_POLY;
                shape.s.poly.count = static_cast<int32_t>(values.size() / 2);
                c2MakePoly(&shape.s.poly);
                shapes.push_back(shape);
            }
        }
        _shapes[shapeId] = std::move(shapes);
    }
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
