#include "renderer/base/atlas.h"

#include "core/ark.h"
#include "core/base/bean_factory.h"
#include "core/base/named_hash.h"
#include "core/dom/dom_document.h"
#include "core/inf/storage.h"
#include "core/util/math.h"
#include "core/util/documents.h"

#include "graphics/base/bitmap.h"
#include "graphics/base/rect.h"

#include "renderer/base/bitmap_bundle.h"
#include "renderer/base/graphics_context.h"
#include "renderer/base/texture.h"
#include "renderer/base/resource_loader_context.h"
#include "renderer/impl/importer/atlas_importer_generic_xml.h"
#include "renderer/impl/vertices/vertices_nine_patch_quads_lhs.h"
#include "renderer/impl/vertices/vertices_nine_patch_quads_rhs.h"
#include "renderer/impl/vertices/vertices_nine_patch_triangle_strips_lhs.h"
#include "renderer/impl/vertices/vertices_nine_patch_triangle_strips_rhs.h"

namespace ark {

namespace {

class GetTextureBitmap final : public Texture::Delegate {
public:
    GetTextureBitmap(sp<Texture::Delegate> delegate)
        : Texture::Delegate(delegate->type()), _delegate(std::move(delegate)) {
    }

    uint64_t id() override {
        return _delegate->id();
    }

    void upload(GraphicsContext& graphicsContext, const sp<Texture::Uploader>& uploader) override {
        _delegate->upload(graphicsContext, uploader);
    }

    ResourceRecycleFunc recycle() override {
        return _delegate->recycle();
    }

    void clear(GraphicsContext& graphicsContext) override {
        _delegate->clear(graphicsContext);
    }

    bool download(GraphicsContext& graphicsContext, Bitmap& bitmap) override {
        return _delegate->download(graphicsContext, bitmap);
    }

    void uploadBitmap(GraphicsContext& /*graphicsContext*/, const Bitmap& bitmap, const Vector<sp<ByteArray>>& imagedata) override {
        _bitmap = sp<Bitmap>::make(bitmap.width(), bitmap.height(), bitmap.rowBytes(), bitmap.channels(), imagedata.at(0));
    }

    const sp<Bitmap>& bitmap() const {
        return _bitmap;
    }

private:
    sp<Texture::Delegate> _delegate;
    sp<Bitmap> _bitmap;
};

constexpr uint32_t UV_NORMALIZE_RANGE = std::numeric_limits<uint16_t>::max();

Rect toUVRect(const Atlas::Item& item)
{
    return Rect(item._uv._ux / static_cast<float>(UV_NORMALIZE_RANGE), item._uv._uy / static_cast<float>(UV_NORMALIZE_RANGE), item._uv._vx / static_cast<float>(UV_NORMALIZE_RANGE), item._uv._vy / static_cast<float>(UV_NORMALIZE_RANGE));
}

}

Atlas::Atlas(sp<Texture> texture)
    : _texture(std::move(texture))
{
    DASSERT(_texture);
}

Atlas::Atlas(sp<Texture> texture, const String& src)
    : Atlas(std::move(texture))
{
    if(src.endsWith(".xml"))
        addImporter(sp<AtlasImporter>::make<AtlasImporterGenericXML>("", 0.5f, 0.5f), Ark::instance().openAsset(src));
    else
        FATAL("Unimplemented atlas importer for this kind of asset: %s", src.c_str());
}

const sp<Texture>& Atlas::texture() const
{
    return _texture;
}

const HashMap<HashId, Atlas::Item>& Atlas::items() const
{
    return _items;
}

HashMap<HashId, Atlas::Item>& Atlas::items()
{
    return _items;
}

uint32_t Atlas::width() const
{
    return static_cast<uint32_t>(_texture->width());
}

uint32_t Atlas::height() const
{
    return static_cast<uint32_t>(_texture->height());
}

bool Atlas::has(const NamedHash& resid) const
{
    return _items.contains(resid.hash());
}

const V2& Atlas::getOriginalSize(const NamedHash& resid) const
{
    return at(resid)._size;
}

const V2& Atlas::getPivot(const NamedHash& resid) const
{
    return at(resid)._pivot;
}

Rect Atlas::getItemUV(const NamedHash& resid) const
{
    return toUVRect(at(resid));
}

sp<BitmapBundle> Atlas::makeBitmapBundle() const
{
    GetTextureBitmap textureBitmap(_texture->delegate());
    GraphicsContext graphicsContext(nullptr, nullptr);
    _texture->uploader()->initialize(graphicsContext, textureBitmap);
    return sp<BitmapBundle>::make(*this, textureBitmap.bitmap());
}

void Atlas::addImporter(const sp<AtlasImporter>& importer, const sp<Readable>& /*readable*/)
{
    importer->import(*this, nullptr);
}

Traits& Atlas::attachments()
{
    return _attachments;
}

void Atlas::add(const int32_t id, const uint32_t ux, const uint32_t uy, const uint32_t vx, const uint32_t vy, const Rect& bounds, const V2& size, const V2& pivot)
{
    const UV uv = toUV(ux, uy, vx, vy, static_cast<uint32_t>(_texture->width()), static_cast<uint32_t>(_texture->height()));
    _items[id] = toItem(uv, bounds, size, pivot);
}

Atlas::UV Atlas::toUV(const uint32_t ux, const uint32_t uy, const uint32_t vx, const uint32_t vy, const uint32_t width, const uint32_t height)
{
    const uint16_t l = unnormalize(ux, width);
    const uint16_t t = unnormalize(uy, height);
    const uint16_t r = unnormalize(vx, width);
    const uint16_t b = unnormalize(vy, height);
    return {l, b, r, t};
}

const Atlas::Item& Atlas::at(const NamedHash& resid) const
{
    const HashId idhash = resid.hash();
    const auto iter = _items.find(resid.hash());
    CHECK(iter != _items.end(), "Item[%u](%s) does not exist", idhash, resid.name().c_str());
    return iter->second;
}

Rect Atlas::getItemBounds(const int32_t id) const
{
    const Item& item = at(id);
    const float nw = _texture->width() / static_cast<float>(UV_NORMALIZE_RANGE);
    const float nh = _texture->height() / static_cast<float>(UV_NORMALIZE_RANGE);
    return {item._uv._ux * nw, item._uv._vy * nh, item._uv._vx * nw, item._uv._uy * nh};
}

uint16_t Atlas::unnormalize(const float v)
{
    return std::min<uint16_t>(v * UV_NORMALIZE_RANGE, UV_NORMALIZE_RANGE);
}

uint16_t Atlas::unnormalize(const uint32_t x, const uint32_t s)
{
    return static_cast<uint16_t>(std::min<uint32_t>(x * UV_NORMALIZE_RANGE / s, UV_NORMALIZE_RANGE));
}

Atlas::Item Atlas::toItem(const UV uv, const Rect& bounds, const V2& size, const V2& pivot)
{
    return {uv, Rect(bounds.left() - pivot.x(), bounds.top() - pivot.y(), bounds.right() - pivot.x(), bounds.bottom() - pivot.y()), size, pivot};
}

void Atlas::AttachmentNinePatch::import(Atlas& atlas, const document& manifest)
{
    const uint32_t textureWidth = atlas.width();
    const uint32_t textureHeight = atlas.height();
    for(const document& i : manifest->children())
    {
        const String name = i->name();
        const Rect paddings = Documents::ensureAttribute<Rect>(i, constants::NINE_PATCH_PADDINGS);
        if(name == "default")
        {
            for(const auto& i : atlas.items())
                add(i.first, textureWidth, textureHeight, paddings, atlas);
        }
        else
        {
            CHECK_WARN(name == "nine-patch", "\"%s\" nodeName should be \"nine-patch\"", Documents::toString(i).c_str());
            const int32_t type = Documents::ensureAttribute<int32_t>(i, constants::TYPE);
            add(type, textureWidth, textureHeight, paddings, atlas);
        }
    }
}

void Atlas::AttachmentNinePatch::add(const int32_t type, const uint32_t textureWidth, const uint32_t textureHeight, const Rect& paddings, const Atlas& atlas)
{
    const Rect bounds = atlas.getItemBounds(type);
    const Rect ninePatches(paddings.left(), paddings.top(), bounds.width() - paddings.right(), bounds.height() - paddings.bottom());
    addNinePatch(type, textureWidth, textureHeight, ninePatches, bounds);
}

void Atlas::AttachmentNinePatch::addNinePatch(const int32_t type, const uint32_t textureWidth, const uint32_t textureHeight, const Rect& ninePatch, const Atlas& atlas)
{
    addNinePatch(type, textureWidth, textureHeight, ninePatch, atlas.getItemBounds(type).translate(1, 1));
}

void Atlas::AttachmentNinePatch::addNinePatch(int32_t type, uint32_t textureWidth, uint32_t textureHeight, const Rect& ninePatch, const Rect& bounds)
{
    _nine_patch_vertices[type] = {
        sp<Vertices>::make<VerticesNinePatchTriangleStripsRHS>(bounds, ninePatch, textureWidth, textureHeight),
        sp<Vertices>::make<VerticesNinePatchQuadsRHS>(bounds, ninePatch, textureWidth, textureHeight),
        sp<Vertices>::make<VerticesNinePatchTriangleStripsLHS>(bounds, ninePatch, textureWidth, textureHeight),
        sp<Vertices>::make<VerticesNinePatchQuadsLHS>(bounds, ninePatch, textureWidth, textureHeight)
    };
}

const sp<Vertices>& Atlas::AttachmentNinePatch::ensureVerticesTriangleStrips(const int32_t type, const bool isLHS) const
{
    const auto iter = _nine_patch_vertices.find(type);
    CHECK(iter != _nine_patch_vertices.end(), "Cannot find type: %d(%s)", type, NamedHash::reverse(type).c_str());
    return isLHS ? iter->second._triangle_strips_lhs : iter->second._triangle_strips_rhs;
}

const sp<Vertices>& Atlas::AttachmentNinePatch::ensureVerticesQuads(int32_t type, bool isLHS) const
{
    const auto iter = _nine_patch_vertices.find(type);
    CHECK(iter != _nine_patch_vertices.end(), "Cannot find type: %d", type);
    return isLHS ? iter->second._quads_lhs : iter->second._quads_rhs;
}

Atlas::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _texture(factory.ensureConcreteClassBuilder<Texture>(manifest, constants::TEXTURE)), _importers(factory.makeBuilderList<AtlasImporter>(manifest, "import"))
{
}

sp<Atlas> Atlas::BUILDER::build(const Scope& args)
{
    sp<Texture> texture = _texture.build(args);
    const sp<Atlas> atlas = sp<Atlas>::make(std::move(texture));

    for(const sp<Builder<AtlasImporter>>& i : _importers)
        i->build(args)->import(atlas, nullptr);

    return atlas;
}

}
