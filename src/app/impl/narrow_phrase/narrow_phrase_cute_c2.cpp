#include "app/impl/narrow_phrase/narrow_phrase_cute_c2.h"

#include "core/types/box.h"
#include "core/util/math.h"

#include "graphics/base/rect.h"
#include "graphics/base/size.h"

#include "renderer/base/resource_loader_context.h"

#include "app/inf/collider.h"
#include "app/util/rigid_body_def.h"

namespace ark {

NarrowPhraseCuteC2::NarrowPhraseCuteC2(const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
{
    float ppu = Documents::getAttribute<float>(manifest, "ppu", 1.0f);

    loadShapes(manifest, ppu);
    for(const document& i : manifest->children("import"))
    {
        const String& src = Documents::ensureAttribute(i, Constants::Attributes::SRC);
        document content = resourceLoaderContext->documents()->get(src);
        loadShapes(content->ensureChild("bodies"), ppu);
    }
}

sp<NarrowPhraseCuteC2::BodyDefCuteC2> NarrowPhraseCuteC2::makeBodyAABB(const Rect& aabb)
{
    return sp<BodyDefCuteC2>::make(V2(aabb.width(), aabb.height()), V2(0.5f), toCuteC2Shapes(makeAABBShapeImpl(aabb)));
}

sp<NarrowPhraseCuteC2::BodyDefCuteC2> NarrowPhraseCuteC2::makeBodyBall(const V2& position, float radius)
{
    return sp<BodyDefCuteC2>::make(V2(radius, radius), V2(0.5f), toCuteC2Shapes(makeBallShapeImpl(position, radius)));
}

sp<NarrowPhraseCuteC2::BodyDefCuteC2> NarrowPhraseCuteC2::makeBodyBox(const Rect& bounds)
{
    return sp<BodyDefCuteC2>::make(V2(bounds.width(), bounds.height()), V2(0.5f), toCuteC2Shapes(makeBoxShapeImpl(bounds)));
}

sp<NarrowPhraseCuteC2::BodyDefCuteC2> NarrowPhraseCuteC2::makeBodyCapsule(const V2& p1, const V2& p2, float radius)
{
    const V2 size = Math::abs(p1 - p2) + V2(radius * 2);
    return sp<BodyDefCuteC2>::make(size, V2(0.5f), toCuteC2Shapes(makeCapsuleShapeImpl(p1, p2, radius)));
}

RigidBodyDef NarrowPhraseCuteC2::makeBodyDef(int32_t shapeId, const sp<Size>& size)
{
    V3 sizeVal;
    sp<BodyDefCuteC2> bodyDef = findBodyDef(shapeId);
    if(bodyDef)
    {
        if(!size)
            sizeVal = V3(bodyDef->size(), 0);
        else
        {
            sizeVal = size->val();
            if(bodyDef->size() != V2(sizeVal.x(), sizeVal.y()))
            {
                bodyDef = sp<BodyDefCuteC2>::make(*bodyDef);
                bodyDef->resize(sizeVal);
            }
        }
    }
    else
    {
        CHECK(size, "Size required for predefined shapes");
        sizeVal = size->val();

        const Rect bounds(sizeVal.x() / -2.0f, sizeVal.y() / -2.0f, sizeVal.x() / 2.0f, sizeVal.y() / 2.0f);
        switch(shapeId)
        {
            case Collider::BODY_SHAPE_BALL:
                bodyDef = makeBodyBall(V2(0, 0), sizeVal.x());
            break;
            case Collider::BODY_SHAPE_AABB:
                bodyDef = makeBodyAABB(bounds);
            break;
            case Collider::BODY_SHAPE_CAPSULE:
            {
                float radius = bounds.width() / 2;
                float x = (bounds.left() + bounds.right()) / 2;
                CHECK(radius < bounds.height() / 2, "Capsule too narrow, width = %.2f, height = %.2f, radius = %.2f", bounds.width(), bounds.height(), radius);
                bodyDef = makeBodyCapsule(V2(x, bounds.top() + radius), V2(x, bounds.bottom() - radius), radius);
                break;
            }
            case Collider::BODY_SHAPE_BOX:
                bodyDef = makeBodyBox(bounds);
            break;
            default:
                FATAL("Shape %d not found", shapeId);
                break;
        }
    }
    return RigidBodyDef(sizeVal, V3(bodyDef->pivot(), 0), bodyDef);
}

NarrowPhrase::Ray NarrowPhraseCuteC2::toRay(const V2& from, const V2& to)
{
    Ray ray;
    toRay(from, to, *ray.data<c2Ray>());
    return ray;
}

bool NarrowPhraseCuteC2::collisionManifold(const BroadPhrase::Candidate& candidateOne, const BroadPhrase::Candidate& candidateOther, CollisionManifold& collisionManifold)
{
    for(const ShapeCuteC2& i : ensureBodyDef(candidateOne)->shapes())
    {
        const ShapeCuteC2 transformed = i.transform(candidateOne._position, candidateOne._rotation);
        const CollisionFilter& oneFilter = getCollisionFilter(transformed._collision_filter, candidateOne._collision_filter);
        for(const ShapeCuteC2& j : ensureBodyDef(candidateOther)->shapes())
            if(oneFilter.collisionTest(getCollisionFilter(j._collision_filter, candidateOther._collision_filter)) &&
                    transformed.collideManifold(j.transform(candidateOther._position, candidateOther._rotation), collisionManifold))
                return true;
    }
    return false;
}

bool NarrowPhraseCuteC2::rayCastManifold(const Ray& ray, const BroadPhrase::Candidate& candidate, RayCastManifold& rayCastManifold)
{
    for(const ShapeCuteC2& i : ensureBodyDef(candidate)->shapes())
    {
        const ShapeCuteC2 transformed = i.transform(candidate._position, candidate._rotation);
        if(transformed.rayCastManifold(*ray.data<c2Ray>(), rayCastManifold))
            return true;
    }
    return false;
}

void NarrowPhraseCuteC2::toRay(const V2& from, const V2& to, c2Ray& ray) const
{
    const V2 delta = to - from;
    ray.p = { from.x(), from.y() };
    if(delta.hypot() > 0.01f)
    {
        const V2 nd = delta.normalize();
        ray.d = { nd.x(), nd.y() };
        ray.t = delta.hypot();
    }
    else
    {
        ray.d = { 0, 1.0f };
        ray.t = 0.01f;
    }
}

void NarrowPhraseCuteC2::loadShapes(const document& manifest, float ppu)
{
    for(const document& i : manifest->children("body"))
    {
        int32_t shapeId = Documents::ensureAttribute<int32_t>(i, "name");
        _body_defs[shapeId] = sp<BodyDefCuteC2>::make(i, ppu);
    }
}

const CollisionFilter& NarrowPhraseCuteC2::getCollisionFilter(const CollisionFilter& oneFilter, const sp<CollisionFilter>& specifiedFilter)
{
    return specifiedFilter ? *specifiedFilter : oneFilter;
}

sp<NarrowPhraseCuteC2::BodyDefCuteC2> NarrowPhraseCuteC2::findBodyDef(int32_t shapeId) const
{
    const auto iter = _body_defs.find(shapeId);
    return iter != _body_defs.end() ? iter->second : nullptr;
}

sp<NarrowPhraseCuteC2::BodyDefCuteC2> NarrowPhraseCuteC2::ensureBodyDef(const BroadPhrase::Candidate& candidate) const
{
    if(candidate._body_def)
        return candidate._body_def.toPtr<BodyDefCuteC2>();
    sp<BodyDefCuteC2> bodyDef = findBodyDef(candidate._shape_id);
    CHECK(bodyDef, "Shape %d not found", candidate._shape_id);
    return bodyDef;
}

std::vector<ShapeCuteC2> NarrowPhraseCuteC2::toCuteC2Shapes(const ShapeCuteC2& shape) const
{
    return std::vector<ShapeCuteC2>{ shape };
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
    CHECK(vertices.size() < C2_MAX_POLYGON_VERTS, "Max polygon vertices exceeded, vertices.size: %d, max size: %d", vertices.size(), C2_MAX_POLYGON_VERTS);

    ShapeCuteC2 shape;
    c2Poly& poly = shape.s.poly;
    shape.t = C2_TYPE_POLY;

    for(size_t i = 0; i < vertices.size(); ++i)
        poly.verts[i] = { vertices.at(i).x(), vertices.at(i).y() };
    poly.count = static_cast<int32_t>(vertices.size());

    c2MakePoly(&poly);
    return shape;
}

NarrowPhraseCuteC2::BodyDefCuteC2::BodyDefCuteC2(const V2& size, const V2& pivot, std::vector<ShapeCuteC2> shapes)
    : _size(size), _pivot(pivot), _shapes(std::move(shapes))
{
}

NarrowPhraseCuteC2::BodyDefCuteC2::BodyDefCuteC2(const document& manifest, float ppu)
    : _size(Documents::getAttribute<float>(manifest, "width", 0) / ppu, Documents::getAttribute<float>(manifest, "height", 0) / ppu), _pivot(0.5f)
{
    const String shapeType = Documents::getAttribute(manifest, "shape-type");
    if(shapeType == "capsule")
    {
        const float ax = Documents::ensureAttribute<float>(manifest, "ax");
        const float ay = Documents::ensureAttribute<float>(manifest, "ay");
        const float bx = Documents::ensureAttribute<float>(manifest, "bx");
        const float by = Documents::ensureAttribute<float>(manifest, "by");
        const float r = Documents::ensureAttribute<float>(manifest, "r");
        _shapes.push_back(NarrowPhraseCuteC2::makeCapsuleShapeImpl(V2(ax, ay) / ppu, V2(bx, by) / ppu, r / ppu));
    }
    else if(shapeType == "polygon")
    {
        int32_t c = 0;
        ShapeCuteC2 shape;
        for(const document& j : manifest->children())
        {
            CHECK(c < C2_MAX_POLYGON_VERTS, "Unable to add more vertex, max count: %d", C2_MAX_POLYGON_VERTS);
            shape.s.poly.verts[c].x = Documents::ensureAttribute<float>(j, "x") / ppu;
            shape.s.poly.verts[c].y = Documents::ensureAttribute<float>(j, "y") / ppu;
            c++;
        }
        shape.t = C2_TYPE_POLY;
        shape.s.poly.count = c;
        c2MakePoly(&shape.s.poly);
        _shapes.push_back(shape);
    }
    else
    {
        const document& anchorpoint = manifest->getChild("anchorpoint");
        const document& fixtures = manifest->ensureChild("fixtures");
        if(anchorpoint)
            _pivot = Strings::parse<V2>(anchorpoint->value());
        for(const document& j : fixtures->children("fixture"))
        {
            const document& fixture_type = j->ensureChild("fixture_type");
            CHECK(fixture_type->value() == "POLYGON", "Unsupported fixture_type: %s", fixture_type->value().c_str());
            const document& polygons = j->ensureChild("polygons");
            for(const document& k : polygons->children("polygon"))
            {
                const std::vector<String> values = k->value().split(',');
                CHECK(values.size() % 2 == 0, "Illegal vertex points: %s", k->value().c_str());
                ShapeCuteC2 shape;
                shape._collision_filter.setCategoryBits(Documents::getValue<uint32_t>(j, "filter_categoryBits", 1));
                shape._collision_filter.setMaskBits(Documents::getValue<uint32_t>(j, "filter_maskBits", 0xffffffff));
                shape._collision_filter.setGroupIndex(Documents::getValue<int32_t>(j, "filter_groupIndex", 0));
                for(size_t l = 0; l < values.size(); l += 2)
                {
                    CHECK(l / 2 < C2_MAX_POLYGON_VERTS, "Unable to add more vertex, max count: %d", C2_MAX_POLYGON_VERTS);
                    shape.s.poly.verts[l / 2].x = Strings::parse<float>(values.at(l)) / ppu;
                    shape.s.poly.verts[l / 2].y = Strings::parse<float>(values.at(l + 1)) / ppu;
                }
                shape.t = C2_TYPE_POLY;
                shape.s.poly.count = static_cast<int32_t>(values.size() / 2);
                c2MakePoly(&shape.s.poly);
                _shapes.push_back(shape);
            }
        }
    }
}

const V2& NarrowPhraseCuteC2::BodyDefCuteC2::size() const
{
    return _size;
}

const V2& NarrowPhraseCuteC2::BodyDefCuteC2::pivot() const
{
    return _pivot;
}

const std::vector<ShapeCuteC2>& NarrowPhraseCuteC2::BodyDefCuteC2::shapes() const
{
    return _shapes;
}

void NarrowPhraseCuteC2::BodyDefCuteC2::resize(const V2& size)
{
    const V2 scale(size.x() / _size.x(), size.y() / _size.y());
    for(ShapeCuteC2& i : _shapes)
        i.resize(scale);
    _size = size;
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
