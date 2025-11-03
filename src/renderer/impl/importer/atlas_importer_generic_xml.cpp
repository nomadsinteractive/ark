#include "renderer/impl/importer/atlas_importer_generic_xml.h"

#include "core/ark.h"
#include "core/types/optional.h"
#include "core/util/documents.h"

#include "graphics/base/rect.h"

#include "renderer/base/atlas.h"

namespace ark {

AtlasImporterGenericXML::AtlasImporterGenericXML(String src, const float px, const float py)
    : _src(std::move(src)), _px(px), _py(py)
{
}

void AtlasImporterGenericXML::import(Atlas& atlas, const sp<Readable>& /*readable*/)
{
    const document src = Documents::loadFromReadable(Ark::instance().openAsset(_src));
    CHECK(src, "Cannot load %s", _src.c_str());
    for(const document& i : src->children())
    {
        const uint32_t x = Documents::getAttribute<uint32_t>(i, "x", 0);
        const uint32_t y = Documents::getAttribute<uint32_t>(i, "y", 0);
        const uint32_t w = Documents::getAttribute<uint32_t>(i, "w", 0);
        const uint32_t h = Documents::getAttribute<uint32_t>(i, "h", 0);
        const uint32_t ox = Documents::getAttribute<uint32_t>(i, "oX", 0);
        const uint32_t oy = Documents::getAttribute<uint32_t>(i, "oY", 0);
        const float ow = static_cast<float>(Documents::getAttribute<uint32_t>(i, "oW", w));
        const float oh = static_cast<float>(Documents::getAttribute<uint32_t>(i, "oH", h));
        const float px = Documents::getAttribute<float>(i, "pX", _px);
        const float py = Documents::getAttribute<float>(i, "pY", _py);
        Rect bounds(static_cast<float>(ox) / ow, static_cast<float>(oy) / oh, static_cast<float>(ox + w) / ow, static_cast<float>(oy + h) / oh);
        bounds.vflip(1.0f);

        const String& n = Documents::ensureAttribute(i, "n");
        const int32_t nid = static_cast<int32_t>(string_hash(n.c_str()));
        atlas.add(nid, x, y, x + w, y + h, bounds, V2(ow, oh), V2(px, 1.0f - py));

        if(const Optional<String> s9 = Documents::getAttributeOptional<String>(i, "s9"))
        {
            const sp<Atlas::AttachmentNinePatch>& aNinePatch = atlas.attachments().ensure<Atlas::AttachmentNinePatch>();
            aNinePatch->addNinePatch(nid, atlas, s9.value());
        }
    }
}

AtlasImporterGenericXML::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _src(factory.ensureBuilder<String>(manifest, constants::SRC)), _px(Documents::getAttribute<float>(manifest, "px", 0.5f)),
      _py(Documents::getAttribute<float>(manifest, "py", 0.5f))
{
}

sp<AtlasImporter> AtlasImporterGenericXML::BUILDER::build(const Scope& args)
{
    return sp<AtlasImporter>::make<AtlasImporterGenericXML>(_src->build(args), _px, _py);
}

}
