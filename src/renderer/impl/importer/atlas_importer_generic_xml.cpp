#include "renderer/impl/importer/atlas_importer_generic_xml.h"

#include "core/ark.h"
#include "core/util/documents.h"

namespace ark {

AtlasImporterGenericXML::AtlasImporterGenericXML(String src, float px, float py)
    : _src(std::move(src)), _px(px), _py(py)
{
}

void AtlasImporterGenericXML::import(Atlas& atlas)
{
    const document src = Documents::loadFromReadable(Ark::instance().openAsset(_src));
    DCHECK(src, "Cannot load %s", _src.c_str());
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
        float px = Documents::getAttribute<float>(i, "pX", _px);
        float py = Documents::getAttribute<float>(i, "pY", _py);
        Rect bounds(ox / ow, oy / oh, (ox + w) / ow, (oy + h) / oh);
        bounds.vflip(1.0f);
        atlas.add(n, x, y, x + w, y + h, bounds, V2(ow, oh), V2(px, 1.0f - py));
    }
}

AtlasImporterGenericXML::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _src(factory.ensureBuilder<String>(manifest, Constants::Attributes::SRC)), _px(Documents::getAttribute<float>(manifest, "px", 0.5f)),
      _py(Documents::getAttribute<float>(manifest, "py", 0.5f))
{
}

sp<Atlas::Importer> AtlasImporterGenericXML::BUILDER::build(const Scope& args)
{
    return sp<AtlasImporterGenericXML>::make(_src->build(args), _px, _py);
}

}
