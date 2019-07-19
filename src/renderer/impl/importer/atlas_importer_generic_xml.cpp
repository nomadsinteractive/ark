#include "renderer/impl/importer/atlas_importer_generic_xml.h"

#include "core/util/documents.h"

#include "renderer/base/atlas.h"
#include "renderer/base/resource_loader_context.h"

namespace ark {

void AtlasImporterTexturePacker::import(Atlas& atlas, const ResourceLoaderContext& resourceLoaderContext, const document& manifest)
{
    const String& path = Documents::ensureAttribute(manifest, Constants::Attributes::SRC);
    const document src = resourceLoaderContext.documents()->get(path);
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
        uint32_t ow = Documents::getAttribute<uint32_t>(i, "oW", w);
        uint32_t oh = Documents::getAttribute<uint32_t>(i, "oH", h);
        float px = Documents::getAttribute<float>(i, "pX", 0);
        float py = Documents::getAttribute<float>(i, "pY", 0);
        atlas.add(n, x, y, x + w, y + h, (ow * px - ox) / w, (h - oh * py + oy) / h);
    }
}

sp<Atlas::Importer> AtlasImporterTexturePacker::DICTIONARY::build(const sp<Scope>& /*args*/)
{
    return sp<AtlasImporterTexturePacker>::make();
}

}
