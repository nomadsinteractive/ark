#include "app/util/body_def_cute_c2.h"

#include "core/util/documents.h"

#include "graphics//base/v2.h"

#include "app/impl/narrow_phrase/narrow_phrase_cute_c2.h"

namespace ark {

BodyDefCuteC2::BodyDefCuteC2(int32_t shapeId, uint32_t width, uint32_t height, std::vector<sp<ShapeCuteC2>> shapes)
    : _shape_id(shapeId), _width(width), _height(height), _shapes(std::move(shapes))
{
}

BodyDefCuteC2::BodyDefCuteC2(const document& manifest)
    : _shape_id(Documents::ensureAttribute<int32_t>(manifest, "name")), _width(Documents::getAttribute<uint32_t>(manifest, "width", 0)), _height(Documents::getAttribute<uint32_t>(manifest, "height", 0))
{
    const String shapeType = Documents::getAttribute(manifest, "shape-type");
    if(shapeType == "capsule")
    {
        const float ax = Documents::ensureAttribute<float>(manifest, "ax");
        const float ay = Documents::ensureAttribute<float>(manifest, "ay");
        const float bx = Documents::ensureAttribute<float>(manifest, "bx");
        const float by = Documents::ensureAttribute<float>(manifest, "by");
        const float r = Documents::ensureAttribute<float>(manifest, "r");
        _shapes.push_back(sp<ShapeCuteC2>::make(NarrowPhraseCuteC2::makeCapsuleShapeImpl(V2(ax, ay), V2(bx, by), r)));
    }
    else if(shapeType == "polygon")
    {
        int32_t c = 0;
        ShapeCuteC2 shape;
        for(const document& j : manifest->children())
        {
            DCHECK(c < C2_MAX_POLYGON_VERTS, "Unable to add more vertex, max count: %d", C2_MAX_POLYGON_VERTS);
            shape.s.poly.verts[c].x = Documents::ensureAttribute<float>(j, "x");
            shape.s.poly.verts[c].y = Documents::ensureAttribute<float>(j, "y");
            c++;
        }
        shape.t = C2_TYPE_POLY;
        shape.s.poly.count = c;
        c2MakePoly(&shape.s.poly);
        _shapes.push_back(sp<ShapeCuteC2>::make(shape));
    }
    else
    {
        const document& fixtures = manifest->ensureChild("fixtures");
        for(const document& j : fixtures->children("fixture"))
        {
            const document& fixture_type = j->ensureChild("fixture_type");
            DCHECK(fixture_type->value() == "POLYGON", "Unsupported fixture_type: %s", fixture_type->value().c_str());
            const document& polygons = j->ensureChild("polygons");
            for(const document& k : polygons->children("polygon"))
            {
                const std::vector<String> values = k->value().split(',');
                DCHECK(values.size() % 2 == 0, "Illegal vertex points: %s", k->value().c_str());
                ShapeCuteC2 shape;
                shape._collision_filter.setCategoryBits(Documents::getValue<uint32_t>(j, "filter_categoryBits", 1));
                shape._collision_filter.setMaskBits(Documents::getValue<uint32_t>(j, "filter_maskBits", 0xffffffff));
                shape._collision_filter.setGroupIndex(Documents::getValue<int32_t>(j, "filter_groupIndex", 0));
                for(size_t k = 0; k < values.size(); k += 2)
                {
                    DCHECK(k / 2 < C2_MAX_POLYGON_VERTS, "Unable to add more vertex, max count: %d", C2_MAX_POLYGON_VERTS);
                    shape.s.poly.verts[k / 2].x = Strings::parse<float>(values.at(k));
                    shape.s.poly.verts[k / 2].y = Strings::parse<float>(values.at(k + 1));
                }
                shape.t = C2_TYPE_POLY;
                shape.s.poly.count = static_cast<int32_t>(values.size() / 2);
                c2MakePoly(&shape.s.poly);
                _shapes.push_back(sp<ShapeCuteC2>::make(shape));
            }
        }
    }
}

int32_t BodyDefCuteC2::shapeId() const
{
    return _shape_id;
}

const std::vector<sp<ShapeCuteC2> >& BodyDefCuteC2::shapes() const
{
    return _shapes;
}

}
