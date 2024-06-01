#include "renderer/impl/importer/atlas_importer_max_rects.h"

#include "core/ark.h"
#include "core/util/math.h"
#include "core/util/documents.h"

#include "graphics/base/bitmap.h"
#include "graphics/util/max_rects_bin_pack.h"

#include "renderer/base/atlas.h"
#include "renderer/base/resource_loader_context.h"
#include "renderer/base/texture_packer.h"
#include "renderer/base/texture.h"

namespace ark {

AtlasImporterMaxRects::AtlasImporterMaxRects(document manifest, sp<ResourceLoaderContext> resourceLoaderContext)
    : _manifest(std::move(manifest)), _resource_loader_context(std::move(resourceLoaderContext))
{
}

void AtlasImporterMaxRects::import(Atlas& atlas, const sp<Readable>& /*readable*/)
{
    TexturePacker texturePacker(Ark::instance().applicationContext(), atlas.texture());
    MaxRectsBinPack binPack(atlas.texture()->width(), atlas.texture()->height(), false);
    for(const document& i : _manifest->children())
    {
        DCHECK(i->name() == "item", "No rule to import item \"%s\"", Documents::toString(i).c_str());

        const String& src = Documents::ensureAttribute(i, constants::SRC);
        int32_t type = Documents::ensureAttribute<int32_t>(i, constants::TYPE);
        const RectI rect = texturePacker.addBitmap(binPack, src);
        atlas.add(type, rect.left(), rect.top(), rect.right(), rect.bottom(), Rect(0, 0, 1.0f, 1.0f), V2(static_cast<float>(rect.width()), static_cast<float>(rect.height())), V2(0.5f, 0.5f));
    }
    texturePacker.updateTexture();
}

AtlasImporterMaxRects::BUILDER::BUILDER(const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _manifest(manifest), _resource_loader_context(resourceLoaderContext)
{
}

sp<AtlasImporter> AtlasImporterMaxRects::BUILDER::build(const Scope& /*args*/)
{
    return sp<AtlasImporterMaxRects>::make(_manifest, _resource_loader_context);
}

}
