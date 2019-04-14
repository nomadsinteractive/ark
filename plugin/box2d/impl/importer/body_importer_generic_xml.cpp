#include "plugin/box2d/impl/importer/body_importer_generic_xml.h"


#include "core/base/bean_factory.h"
#include "core/inf/dictionary.h"
#include "core/util/strings.h"

#include "graphics/base/v2.h"

#include "renderer/base/atlas.h"
#include "renderer/base/resource_loader_context.h"

#include "box2d/impl/body_create_info.h"
#include "box2d/inf/shape.h"

namespace ark {
namespace plugin {
namespace box2d {

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
                float x = Strings::parse<float>(splitted.at(j)) / size.x();
                float y = Strings::parse<float>(splitted.at(j + 1)) / size.y();
                vec2s.push_back(V2(x, y));
            }
            _polygons.push_back(std::move(vec2s));
        }
    }

    virtual void apply(b2Body* body, const sp<Size>& size, const BodyCreateInfo& createInfo) override {
        for(const std::vector<V2>& i : _polygons) {
            b2PolygonShape shape;

            std::vector<b2Vec2> vec2s;
            float width = size->width(), height = size->height();
            for(const V2& j : i)
                vec2s.push_back(b2Vec2(j.x() * width, j.y() * height));

            shape.Set(vec2s.data(), static_cast<int32_t>(vec2s.size()));

            b2FixtureDef fixtureDef = createInfo.toFixtureDef(&shape);
            body->CreateFixture(&fixtureDef);
        }
    }

private:
    std::vector<std::vector<V2>> _polygons;

};

}

ImporterGenericXML::ImporterGenericXML(const document& manifest)
    : _manifest(manifest)
{
}

void ImporterGenericXML::import(World& world)
{
    const document bodies = _manifest->getChild("bodies");
    DASSERT(bodies);

    for(const document& i : bodies->children("body"))
    {
        const String& name = Documents::ensureAttribute(i, Constants::Attributes::NAME);
        int32_t id = Strings::parse<int32_t>(name);
        DCHECK(id >= 0, "Illegal body id: %d, \"%s\" must be a postive integer", id, name.c_str());

        const String& ss = i->ensureChild("size")->value();
        const std::vector<String> sst = ss.split(',');
        DCHECK(sst.size() == 2, "Illegal size: %s", ss.c_str());
        const V2 size(Strings::parse<float>(sst.at(0)), Strings::parse<float>(sst.at(1)));

        const document& fixtures = i->ensureChild("fixtures");
        const document& fixture = fixtures->ensureChild("fixture");
        const document& fixture_type = fixture->ensureChild("fixture_type");
        DCHECK(fixture_type->value() == "POLYGON", "Unsupported fixture_type: %s", fixture_type->value().c_str());

        const document& density = fixture->ensureChild("density");
        const document& friction = fixture->ensureChild("friction");

        const sp<ShapePolygon> polygons = sp<ShapePolygon>::make(fixture->ensureChild("polygons"), size);
        BodyCreateInfo bodyManifest(polygons, Strings::parse<float>(density->value()), Strings::parse<float>(friction->value()));
        bodyManifest.category = Strings::parse<uint16_t>(fixture->ensureChild("filter_categoryBits")->value());
        bodyManifest.group = Strings::parse<int16_t>(fixture->ensureChild("filter_groupIndex")->value());
        bodyManifest.mask = Strings::parse<uint16_t>(fixture->ensureChild("filter_maskBits")->value());
        bodyManifest.is_sensor = fixture->getChild("isSensor") != document::null();
        world.setBodyManifest(id, bodyManifest);
    }
}

ImporterGenericXML::BUILDER::BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _resource_loader_context(resourceLoaderContext), _src(factory.ensureBuilder<String>(manifest, Constants::Attributes::SRC))
{
}

sp<World::Importer> ImporterGenericXML::BUILDER::build(const sp<Scope>& args)
{
    const sp<String> src = _src->build(args);
    const document manifest = _resource_loader_context->documents()->get(src);
    return sp<ImporterGenericXML>::make(manifest);
}

}
}
}
