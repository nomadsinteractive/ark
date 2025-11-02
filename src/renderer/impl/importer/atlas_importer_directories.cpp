#include "renderer/impl/importer/atlas_importer_directories.h"


#include "core/ark.h"
#include "core/impl/dictionary/loader_bundle.h"
#include "core/util/documents.h"

#include "graphics/base/bitmap.h"

#include "renderer/base/atlas.h"
#include "renderer/base/texture_packer.h"
#include "renderer/base/texture.h"

#include "app/base/application_bundle.h"
#include "app/base/application_context.h"

namespace ark {

namespace {

class BitmapProvider final : public Variable<bitmap> {
public:
    BitmapProvider(sp<BitmapLoaderBundle> bitmapBundle, String src)
        : _bitmap_bundle(std::move(bitmapBundle)), _src(std::move(src)) {

    }

    bool update(uint32_t /*tick*/) override {
        return false;
    }

    bitmap val() override {
        return _bitmap_bundle->get(_src);
    }

private:
    sp<BitmapLoaderBundle> _bitmap_bundle;
    String _src;
};

}

AtlasImporterDirectories::AtlasImporterDirectories(Vector<String> directories)
    : _directories(std::move(directories))
{
}

void AtlasImporterDirectories::import(Atlas& atlas, const sp<Readable>& /*readable*/)
{
    TexturePacker texturePacker(atlas.texture()->width(), atlas.texture()->height());
    const ApplicationBundle& applicationBundle = Ark::instance().applicationContext()->applicationBundle();
    const sp<BitmapLoaderBundle> bitmapLoader = applicationBundle.bitmapBundle();
    BitmapLoaderBundle& bitmapLoaderBounds = applicationBundle.bitmapBoundsBundle();
    for(const String& i : _directories)
    {
        const sp<AssetBundle> assetBundle = Ark::instance().getAssetBundle(i);
        for(const String& j : assetBundle->listAssets())
            if(j.endsWith(".png"))
            {
                auto [name, ext] = j.rcut('.');
                String src = Strings::sprintf("%s/%s", i.c_str(), j.c_str());
                sp<Bitmap> bounds = bitmapLoaderBounds.get(src);
                texturePacker.addBitmap(std::move(bounds), sp<Variable<bitmap>>::make<BitmapProvider>(bitmapLoader, std::move(src)), std::move(name.value()));
            }
    }
    for(const auto& [name, _, __, uv] : texturePacker.packedBitmaps())
        atlas.add(string_hash(name.c_str()), uv.left(), uv.top(), uv.right(), uv.bottom(), Rect(0, 0, 1.0f, 1.0f), V2(static_cast<float>(uv.width()), static_cast<float>(uv.height())), V2(0.5f, 0.5f));
    texturePacker.updateTexture(atlas.texture());
}

AtlasImporterDirectories::BUILDER::BUILDER(const document& manifest)
{
    for(const document& i : manifest->children("directory"))
        _directories.emplace_back(Documents::ensureAttribute(i, constants::SRC));
}

sp<AtlasImporter> AtlasImporterDirectories::BUILDER::build(const Scope& /*args*/)
{
    return sp<AtlasImporter>::make<AtlasImporterDirectories>(_directories);
}

}
