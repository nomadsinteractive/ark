#include "renderer/impl/atlas_importer/atlas_importer_texture_packer.h"

#include "core/util/documents.h"

#include "renderer/base/atlas.h"
#include "renderer/base/gl_texture.h"
#include "renderer/base/resource_loader_context.h"

namespace ark {

void AtlasImporterTexturePacker::import(Atlas& atlas, const ResourceLoaderContext& resourceLoaderContext, const document& manifest)
{
    const String& path = Documents::ensureAttribute(manifest, Constants::Attributes::SRC);
    const document src = resourceLoaderContext.documents()->get(path);
    DCHECK(src, "Cannot load %s", path.c_str());
    for(const document& i : src->children())
    {
        uint32_t n = Documents::ensureAttribute<uint32_t>(i, "n");
        uint32_t x = Documents::getAttribute<uint32_t>(i, "x", 0);
        uint32_t y = Documents::getAttribute<uint32_t>(i, "y", 0);
        uint32_t w = Documents::getAttribute<uint32_t>(i, "w", 0);
        uint32_t h = Documents::getAttribute<uint32_t>(i, "h", 0);
        float px = Documents::getAttribute<float>(i, "pX", 0);
        float py = Documents::getAttribute<float>(i, "pY", 0);
        atlas.add(n, x, y, x + w, y + h, px, 1.0f - py);
    }
}

sp<Atlas::Importer> AtlasImporterTexturePacker::DICTIONARY::build(const sp<Scope>& /*args*/)
{
    return sp<AtlasImporterTexturePacker>::make();
}



}
