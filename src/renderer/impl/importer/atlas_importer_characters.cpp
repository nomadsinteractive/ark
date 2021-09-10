#include "renderer/impl/importer/atlas_importer_characters.h"

#include "core/util/documents.h"

#include "renderer/base/atlas.h"
#include "renderer/base/texture.h"

namespace ark {

AtlasImporterCharacters::AtlasImporterCharacters(String characters, uint32_t fontWidth, uint32_t fontHeight)
    : _characters(std::move(characters)), _font_width(fontWidth), _font_height(fontHeight)
{
}

void AtlasImporterCharacters::import(Atlas& atlas, const sp<Readable>& /*readable*/)
{
    uint32_t flowx = 0;
    uint32_t flowy = 0;
    uint32_t textureWidth = atlas.width();
    for(const char* iter = _characters.c_str(); *iter; iter++)
    {
        atlas.add(*iter, flowx, flowy, flowx + _font_width, flowy + _font_height, Rect(0, 0, 1, 1), V2(static_cast<float>(_font_width), static_cast<float>(_font_height)), V3(0));
        flowx += _font_width;
        if(flowx >= textureWidth)
        {
            flowx = 0;
            flowy += _font_height;
        }
    }
}

AtlasImporterCharacters::BUILDER::BUILDER(BeanFactory& /*factory*/, const document& manifest)
    : _characters(Documents::ensureAttribute(manifest, "characters")), _font_width(Documents::ensureAttribute<uint32_t>(manifest, "font-width")),
      _font_height(Documents::ensureAttribute<uint32_t>(manifest, "font-height"))
{
}

sp<AtlasImporter> AtlasImporterCharacters::BUILDER::build(const Scope& /*args*/)
{
    return sp<AtlasImporterCharacters>::make(_characters, _font_width, _font_height);
}



}
