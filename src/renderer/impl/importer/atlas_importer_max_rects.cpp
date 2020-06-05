#include "renderer/impl/importer/atlas_importer_max_rects.h"

#include "core/util/math.h"
#include "core/util/documents.h"

#include "graphics/base/bitmap.h"
#include "graphics/base/bitmap_bundle.h"
#include "graphics/util/max_rects_bin_pack.h"

#include "renderer/base/atlas.h"
#include "renderer/base/resource_loader_context.h"
#include "renderer/base/texture_packer.h"
#include "renderer/base/texture.h"

namespace ark {

AtlasImporterMaxRects::AtlasImporterMaxRects(const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _resource_loader_context(resourceLoaderContext)
{
}

void AtlasImporterMaxRects::import(Atlas& atlas, BeanFactory& factory, const document& manifest)
{
    const sp<Texture>& texture = atlas.texture();
    TexturePacker texturePacker(_resource_loader_context, texture->width(), texture->height(), false);
    for(const document& i : manifest->children())
    {
        if(i->name() == Constants::Attributes::ATLAS)
        {
            Atlas imported(nullptr);
            const sp<Atlas::Importer> importer = factory.ensure<Atlas::Importer>(i, Scope());
            importer->import(imported, factory, i);
        }
        else
        {
            DCHECK(i->name() == "item", "No rule to import item \"%s\"", Documents::toString(i).c_str());

            const String& src = Documents::ensureAttribute(i, Constants::Attributes::SRC);
            int32_t type = Documents::ensureAttribute<int32_t>(i, Constants::Attributes::TYPE);
            const RectI rect = texturePacker.addBitmap(src);
            atlas.add(type, rect.left(), rect.top(), rect.right(), rect.bottom(), Rect(0, 0, 1.0f, 1.0f), V2(rect.width(), rect.height()), V2(0.5f, 0.5f));
        }
    }
    texturePacker.updateTexture(texture);
}

AtlasImporterMaxRects::BUILDER::BUILDER(const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _resource_loader_context(resourceLoaderContext)
{
}

sp<Atlas::Importer> AtlasImporterMaxRects::BUILDER::build(const Scope& /*args*/)
{
    return sp<AtlasImporterMaxRects>::make(_resource_loader_context);
}

}
