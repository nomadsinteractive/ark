#include "renderer/impl/importer/atlas_importer_impl.h"

#include "core/ark.h"
#include "core/base/named_hash.h"
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
    for(const auto& [dirpath] : _directories)
    {
        const sp<AssetBundle> assetBundle = Ark::instance().getAssetBundle(dirpath);
        for(const String& j : assetBundle->listAssets(""))
            if(j.endsWith(".png"))
            {
                auto [name, ext] = j.rcut('.');
                String src = Strings::sprintf("%s/%s", dirpath.c_str(), j.c_str());
                sp<Bitmap> bounds = bitmapLoaderBounds.get(src);
                texturePacker.addBitmap(std::move(bounds), sp<Variable<bitmap>>::make<BitmapProvider>(bitmapLoader, std::move(src)), std::move(name.value()));
            }
    }

    texturePacker.updateTexture(atlas.texture());

    const Rect bounds = Rect(0, 0, 1.0f, 1.0f).vflip(1.0f);
    for(const auto& [name, _, __, uv] : texturePacker.packedBitmaps())
        atlas.add(string_hash(name.c_str()), uv.left(), uv.top(), uv.right(), uv.bottom(), bounds, V2(static_cast<float>(uv.width()), static_cast<float>(uv.height())), V2(0.5f, 0.5f));

    for(const auto& [manifestSrc, fileSrc] : _files)
    {
        const document manifest = applicationBundle.loadDocument(manifestSrc);
        CHECK(manifest, "Unable to load manifest \"%s\"", manifestSrc.c_str());
        for(const document& j : manifest->children())
        {
            const String& n = Documents::ensureAttribute(j, "n");
            if(const HashId nid = string_hash(n.c_str()); atlas.has(nid))
            {
                const uint32_t w = Documents::getAttribute<uint32_t>(j, "w", 0);
                const uint32_t h = Documents::getAttribute<uint32_t>(j, "h", 0);
                const uint32_t ox = Documents::getAttribute<uint32_t>(j, "oX", 0);
                const uint32_t oy = Documents::getAttribute<uint32_t>(j, "oY", 0);
                const float ow = static_cast<float>(Documents::getAttribute<uint32_t>(j, "oW", w));
                const float oh = static_cast<float>(Documents::getAttribute<uint32_t>(j, "oH", h));
                const float px = Documents::getAttribute<float>(j, "pX", 0.5f);
                const float py = Documents::getAttribute<float>(j, "pY", 0.5f);
                const V2 pivot((ox + px * w) / ow, 1.0f - (oy + py * h) / oh);
                Atlas::Item& item = atlas.at(nid);
                item._pivot = pivot;
                item._bounds = item._bounds.translate(0.5f, 0.5f).translate(-pivot.x(), -pivot.y());

                if(const Optional<String> s9 = Documents::getAttributeOptional<String>(j, "s9"))
                {
                    const sp<Atlas::AttachmentNinePatch>& aNinePatch = atlas.attachments().ensure<Atlas::AttachmentNinePatch>();
                    aNinePatch->addNinePatch(nid, atlas, s9.value());
                }
            }
        }
    }
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
