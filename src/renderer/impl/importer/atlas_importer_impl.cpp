#include "renderer/impl/importer/atlas_importer_impl.h"

#include "core/ark.h"
#include "core/impl/dictionary/loader_bundle.h"
#include "core/util/documents.h"

#include "graphics/base/bitmap.h"

#include "renderer/base/atlas.h"
#include "renderer/base/texture_packer.h"
#include "renderer/base/texture.h"

#include "app/base/application_bundle.h"
#include "app/base/application_context.h"
#include "core/base/named_hash.h"

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

AtlasImporterImpl::AtlasImporterImpl(Vector<Directory> directories, Vector<File> files)
    : _directories(std::move(directories)), _files(std::move(files))
{
}

void AtlasImporterImpl::import(Atlas& atlas, const sp<Readable>& /*readable*/)
{
    TexturePacker texturePacker(atlas.texture()->width(), atlas.texture()->height());
    const ApplicationBundle& applicationBundle = Ark::instance().applicationContext()->applicationBundle();
    const sp<BitmapLoaderBundle> bitmapLoader = applicationBundle.bitmapBundle();
    BitmapLoaderBundle& bitmapLoaderBounds = applicationBundle.bitmapBoundsBundle();
    for(const Directory& i : _directories)
    {
        const sp<AssetBundle> assetBundle = Ark::instance().getAssetBundle(i._src);
        for(const String& j : assetBundle->listAssets())
            if(j.endsWith(".png"))
            {
                auto [name, ext] = j.rcut('.');
                String src = Strings::sprintf("%s/%s", i._src.c_str(), j.c_str());
                sp<Bitmap> bounds = bitmapLoaderBounds.get(src);
                texturePacker.addBitmap(std::move(bounds), sp<Variable<bitmap>>::make<BitmapProvider>(bitmapLoader, std::move(src)), std::move(name.value()));
            }
    }
    const Rect bounds = Rect(0, 0, 1.0f, 1.0f).vflip(1.0f);
    for(const auto& [name, _, __, uv] : texturePacker.packedBitmaps())
        atlas.add(string_hash(name.c_str()), uv.left(), uv.top(), uv.right(), uv.bottom(), bounds, V2(static_cast<float>(uv.width()), static_cast<float>(uv.height())), V2(0.5f, 0.5f));

    for(const File& i : _files)
    {
        const document manifest = applicationBundle.loadDocument(i._manifest);
        CHECK(manifest, "Unable to load manifest \"%s\"", i._manifest.c_str());
        for(const document& j : manifest->children())
        {
            const String& n = Documents::ensureAttribute(j, "n");
            const HashId nid = string_hash(n.c_str());
            if(const Optional<String> s9 = Documents::getAttributeOptional<String>(j, "s9"); s9 && atlas.has(NamedHash(nid)))
            {
                const sp<Atlas::AttachmentNinePatch>& aNinePatch = atlas.attachments().ensure<Atlas::AttachmentNinePatch>();
                aNinePatch->addNinePatch(nid, atlas, s9.value());
            }
        }
    }

    texturePacker.updateTexture(atlas.texture());
}

AtlasImporterImpl::BUILDER::BUILDER(const document& manifest)
{
    for(const document& i : manifest->children("directory"))
        _directories.emplace_back(Documents::ensureAttribute(i, constants::SRC));
    for(const document& i : manifest->children("file"))
        _files.emplace_back(Documents::ensureAttribute(i, "manifest"), Documents::getAttribute(i, constants::SRC));
}

sp<AtlasImporter> AtlasImporterImpl::BUILDER::build(const Scope& /*args*/)
{
    return sp<AtlasImporter>::make<AtlasImporterImpl>(_directories, _files);
}

}
