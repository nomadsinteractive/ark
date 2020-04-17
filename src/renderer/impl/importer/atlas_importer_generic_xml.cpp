#include "renderer/impl/importer/atlas_importer_generic_xml.h"

#include "core/ark.h"
#include "core/util/documents.h"

namespace ark {

void AtlasImporterGenericXML::import(Atlas& atlas, const document& manifest)
{
    const String& path = Documents::ensureAttribute(manifest, Constants::Attributes::SRC);
    const document src = Documents::loadFromReadable(Ark::instance().openAsset(path));
    float defPx = Documents::getAttribute<float>(manifest, "px", 0.5f);
    float defPy = Documents::getAttribute<float>(manifest, "py", 0.5f);
    DCHECK(src, "Cannot load %s", path.c_str());
    for(const document& i : src->children())
    {
        int32_t n = Documents::ensureAttribute<int32_t>(i, "n");
        uint32_t x = Documents::getAttribute<uint32_t>(i, "x", 0);
        uint32_t y = Documents::getAttribute<uint32_t>(i, "y", 0);
        uint32_t w = Documents::getAttribute<uint32_t>(i, "w", 0);
        uint32_t h = Documents::getAttribute<uint32_t>(i, "h", 0);
        uint32_t ox = Documents::getAttribute<uint32_t>(i, "oX", 0);
        uint32_t oy = Documents::getAttribute<uint32_t>(i, "oY", 0);
        float ow = static_cast<float>(Documents::getAttribute<uint32_t>(i, "oW", w));
        float oh = static_cast<float>(Documents::getAttribute<uint32_t>(i, "oH", h));
        float px = Documents::getAttribute<float>(i, "pX", defPx);
        float py = Documents::getAttribute<float>(i, "pY", defPy);
        Rect bounds(ox / ow, oy / oh, (ox + w) / ow, (oy + h) / oh);
        bounds.vflip(1.0f);
        atlas.add(n, x, y, x + w, y + h, bounds, V2(ow, oh), V2(px, 1.0f - py));
    }
}

sp<Atlas::Importer> AtlasImporterGenericXML::BUILDER::build(const Scope& /*args*/)
{
    return sp<AtlasImporterGenericXML>::make();
}

}
