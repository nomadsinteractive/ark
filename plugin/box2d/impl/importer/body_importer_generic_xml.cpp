#include "plugin/box2d/impl/importer/body_importer_generic_xml.h"


#include "core/base/bean_factory.h"
#include "core/inf/dictionary.h"
#include "core/util/strings.h"

#include "graphics/base/v2.h"

#include "renderer/base/atlas.h"
#include "renderer/base/resource_loader_context.h"

#include "box2d/impl/body_create_info.h"
#include "box2d/inf/shape.h"

namespace ark::plugin::box2d {

namespace {

class ShapePolygon : public Shape {
public:
    ShapePolygon(const document& polygons, const V2& size) {
        for(const document& i : polygons->children("polygon")) {
            const String& content = i->value();
            std::vector<String> splitted = content.split(',');
            DCHECK(splitted.size() % 2 == 0, "Illegal size: %d", splitted.size());

            std::vector<V2> vec2s;
            for(size_t j = 0; j < splitted.size(); j += 2) {
                float x = Strings::eval<float>(splitted.at(j)) / size.x();
                float y = Strings::eval<float>(splitted.at(j + 1)) / size.y();
                vec2s.push_back(V2(x, y));
            }
            _polygons.push_back(std::move(vec2s));
        }
    }

    void apply(b2Body* body, const V3& size, const BodyCreateInfo& createInfo) override {
        for(const std::vector<V2>& i : _polygons) {
            b2PolygonShape shape;

            std::vector<b2Vec2> vec2s;
            for(const V2& j : i)
                vec2s.push_back(b2Vec2(j.x() * size.x(), j.y() * size.y()));

            shape.Set(vec2s.data(), static_cast<int32_t>(vec2s.size()));

            b2FixtureDef fixtureDef = createInfo.toFixtureDef(&shape);
            body->CreateFixture(&fixtureDef);
        }
    }

private:
    std::vector<std::vector<V2>> _polygons;

};

}

void RigidBodyImporterGenericXML::import(ColliderBox2D& world, const sp<Readable>& readable)
{
    const document manifest = Documents::loadFromReadable(readable);
    const document bodies = manifest->getChild("bodies");
    DASSERT(bodies);

    for(const document& i : bodies->children("body"))
    {
        const String& name = Documents::ensureAttribute(i, Constants::Attributes::NAME);
        int32_t id = Strings::eval<int32_t>(name);
        DCHECK(id >= 0, "Illegal body id: %d, \"%s\" must be a postive integer", id, name.c_str());

        const V2 size = Strings::eval<V2>(i->ensureChild("size")->value());
        const document& fixtures = i->ensureChild("fixtures");
        const document& fixture = fixtures->ensureChild("fixture");
        const document& fixture_type = fixture->ensureChild("fixture_type");
        DCHECK(fixture_type->value() == "POLYGON", "Unsupported fixture_type: %s", fixture_type->value().c_str());

        const document& density = fixture->ensureChild("density");
        const document& friction = fixture->ensureChild("friction");

        const sp<ShapePolygon> polygons = sp<ShapePolygon>::make(fixture->ensureChild("polygons"), size);
        BodyCreateInfo bodyManifest(polygons, Strings::eval<float>(density->value()), Strings::eval<float>(friction->value()));
        bodyManifest.category = Strings::eval<uint16_t>(fixture->ensureChild("filter_categoryBits")->value());
        bodyManifest.group = Strings::eval<int16_t>(fixture->ensureChild("filter_groupIndex")->value());
        bodyManifest.mask = Strings::eval<uint16_t>(fixture->ensureChild("filter_maskBits")->value());
        bodyManifest.is_sensor = fixture->getChild("isSensor") != nullptr;
        world.setBodyManifest(id, bodyManifest);
    }
}

sp<ColliderBox2D::RigidBodyImporter> RigidBodyImporterGenericXML::BUILDER::build(const Scope& /*args*/)
{
    return sp<RigidBodyImporterGenericXML>::make();
}

}
