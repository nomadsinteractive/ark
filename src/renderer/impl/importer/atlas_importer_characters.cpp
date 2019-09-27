#include "renderer/impl/importer/atlas_importer_characters.h"

#include "core/util/documents.h"

#include "renderer/base/atlas.h"
#include "renderer/base/texture.h"

namespace ark {

void AtlasImporterCharacters::import(Atlas& atlas, const document& manifest)
{
    uint32_t flowx = 0;
    uint32_t flowy = 0;
    uint32_t fontWidth = Documents::ensureAttribute<uint32_t>(manifest, "font-width");
    uint32_t fontHeight = Documents::ensureAttribute<uint32_t>(manifest, "font-height");
    uint32_t textureWidth = static_cast<uint32_t>(atlas.texture()->width());
    const String& characters = Documents::ensureAttribute(manifest, "characters");
    for(const char* iter = characters.c_str(); *iter; iter++)
    {
        atlas.add(*iter, flowx, flowy, flowx + fontWidth, flowy + fontHeight);
        flowx += fontWidth;
        if(flowx >= textureWidth)
        {
            flowx = 0;
            flowy += fontHeight;
        }
    }
}

sp<Atlas::Importer> AtlasImporterCharacters::BUILDER::build(const Scope& /*args*/)
{
    return sp<AtlasImporterCharacters>::make();
}



}
