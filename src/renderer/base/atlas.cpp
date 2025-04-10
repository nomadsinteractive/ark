#include "renderer/base/atlas.h"

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

    void uploadBitmap(GraphicsContext& /*graphicsContext*/, const Bitmap& bitmap, const std::vector<ark::sp<ByteArray>>& imagedata) override {
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
    return Rect(item._ux / static_cast<float>(UV_NORMALIZE_RANGE), item._uy / static_cast<float>(UV_NORMALIZE_RANGE), item._vx / static_cast<float>(UV_NORMALIZE_RANGE), item._vy / static_cast<float>(UV_NORMALIZE_RANGE));
}

}

Atlas::Atlas(sp<Texture> texture)
    : _texture(std::move(texture))
{
}

void Atlas::loadItem(const document& manifest)
{
    CHECK(manifest->name() == "item", "No rule to import item \"%s\"", Documents::toString(manifest).c_str());
    const int32_t type = Documents::getAttribute<int32_t>(manifest, constants::TYPE, 0);
    const float px = Documents::getAttribute<float>(manifest, "pivot-x", 0);
    const float py = Documents::getAttribute<float>(manifest, "pivot-x", 0);
    if(has(type))
    {
        const Item& item = at(type);
        _items[type] = {item._ux, item._uy, item._vx, item._vy, Rect(-px, -py, 1.0f - px, 1.0f - py), item._size, V2(px, py)};
    }
    else
    {
        const Rect r = Rect::parse(manifest);
        add(type, static_cast<uint32_t>(r.left()), static_cast<uint32_t>(r.top()), static_cast<uint32_t>(r.right()), static_cast<uint32_t>(r.bottom()),
            Rect(0, 0, 1.0f, 1.0f), V2(r.width(), r.height()), V2(px, py));
    }
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
    return _items.find(resid.hash()) != _items.end();
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

void Atlas::add(int32_t id, uint32_t ux, uint32_t uy, uint32_t vx, uint32_t vy, const Rect& bounds, const V2& size, const V2& pivot)
{
    _items[id] = makeItem(ux, uy, vx, vy, bounds, size, pivot);
}

const Atlas::Item& Atlas::at(const NamedHash& resid) const
{
    const HashId idhash = resid.hash();
    const auto iter = _items.find(resid.hash());
    CHECK(iter != _items.end(), "Item[%u](%s) does not exist", idhash, resid.name().c_str());
    return iter->second;
}

Rect Atlas::getItemBounds(int32_t id) const
{
    const Item& item = at(id);
    const float nw = _texture->width() / static_cast<float>(UV_NORMALIZE_RANGE);
    const float nh = _texture->height() / static_cast<float>(UV_NORMALIZE_RANGE);
    return {item._ux * nw, item._vy * nh, item._vx * nw, item._uy * nh};
}

uint16_t Atlas::unnormalize(float v)
{
    return std::min<uint16_t>(v * UV_NORMALIZE_RANGE, UV_NORMALIZE_RANGE);
}

uint16_t Atlas::unnormalize(uint32_t x, uint32_t s)
{
    return static_cast<uint16_t>(std::min<uint32_t>(x * UV_NORMALIZE_RANGE / s, UV_NORMALIZE_RANGE));
}

Atlas::Item Atlas::makeItem(uint32_t ux, uint32_t uy, uint32_t vx, uint32_t vy, const Rect& bounds, const V2& size, const V2& pivot) const
{
    const uint16_t l = unnormalize(ux, static_cast<uint32_t>(_texture->width()));
    const uint16_t t = unnormalize(uy, static_cast<uint32_t>(_texture->height()));
    const uint16_t r = unnormalize(vx, static_cast<uint32_t>(_texture->width()));
    const uint16_t b = unnormalize(vy, static_cast<uint32_t>(_texture->height()));
    return {l, b, r, t, Rect(bounds.left() - pivot.x(), bounds.top() - pivot.y(), bounds.right() - pivot.x(), bounds.bottom() - pivot.y()), size, pivot};
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

void Atlas::AttachmentNinePatch::add(int32_t type, uint32_t textureWidth, uint32_t textureHeight, const Rect& paddings, const Atlas& atlas)
{
    const Rect bounds = atlas.getItemBounds(type);
    const Rect ninePatches(paddings.left(), paddings.top(), bounds.width() - paddings.right(), bounds.height() - paddings.bottom());
    addNinePatch(type, textureWidth, textureHeight, ninePatches, bounds);
}

void Atlas::AttachmentNinePatch::addNinePatch(int32_t type, uint32_t textureWidth, uint32_t textureHeight, const Rect& ninePatch, const Atlas& atlas)
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
    DASSERT(texture);
    const sp<Atlas> atlas = sp<Atlas>::make(std::move(texture));

    for(const sp<Builder<AtlasImporter>>& i : _importers)
        i->build(args)->import(atlas, nullptr);

    return atlas;
}

}
