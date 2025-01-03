#pragma once

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/base/bean_factory.h"
#include "core/collection/traits.h"
#include "core/inf/builder.h"
#include "core/types/safe_builder.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/base/rect.h"
#include "graphics/base/v2.h"

#include "renderer/forwarding.h"

namespace ark {

class ARK_API Atlas {
public:
    Atlas(sp<Texture> texture);

    struct Item {
        uint16_t _ux, _uy;
        uint16_t _vx, _vy;
        Rect _bounds;
        V2 _size;
        V2 _pivot;
    };

    void loadItem(const document& manifest);

//  [[script::bindings::property]]
    const sp<Texture>& texture() const;

//  [[script::bindings::property]]
    uint32_t width() const;
//  [[script::bindings::property]]
    uint32_t height() const;

//  [[script::bindings::auto]]
    bool has(const NamedHash& resid) const;
//  [[script::bindings::auto]]
    const V2& getOriginalSize(const NamedHash& resid) const;
//  [[script::bindings::auto]]
    const V2& getPivot(const NamedHash& resid) const;
//  [[script::bindings::auto]]
    Rect getItemUV(const NamedHash& resid) const;

//  [[script::bindings::auto]]
    sp<BitmapBundle> makeBitmapBundle() const;

//  [[script::bindings::auto]]
    void addImporter(const sp<AtlasImporter>& importer, const sp<Readable>& readable = nullptr);

    Traits& attachments();

    const std::unordered_map<HashId, Item>& items() const;
    std::unordered_map<HashId, Item>& items();

    void add(int32_t id, uint32_t ux, uint32_t uy, uint32_t vx, uint32_t vy, const Rect& bounds, const V2& size, const V2& pivot);
    Item makeItem(uint32_t ux, uint32_t uy, uint32_t vx, uint32_t vy, const Rect& bounds, const V2& size, const V2& pivot) const;

    const Item& at(const NamedHash& resid) const;
    Rect getItemBounds(int32_t id) const;

    static uint16_t unnormalize(float v);
    static uint16_t unnormalize(uint32_t x, uint32_t s);

    class AttachmentNinePatch {
    public:
        const sp<Vertices>& ensureVerticesTriangleStrips(int32_t type) const;
        const sp<Vertices>& ensureVerticesQuads(int32_t type) const;

        void import(Atlas& atlas, const document& manifest);

        void add(int32_t type, uint32_t textureWidth, uint32_t textureHeight, const Rect& paddings, const Atlas& atlas);
        void addNinePatch(int32_t type, uint32_t textureWidth, uint32_t textureHeight, const Rect& ninePatch, const Atlas& atlas);

    private:
        void addNinePatch(int32_t type, uint32_t textureWidth, uint32_t textureHeight, const Rect& ninePatch, const Rect& bounds);

        sp<Vertices> makeNinePatchVertices(uint32_t textureWidth, uint32_t textureHeight, const Rect& paddings, const Rect& bounds) const;

    private:
        std::unordered_map<int32_t, sp<Vertices>> _vertices_triangle_strips;
        std::unordered_map<int32_t, sp<Vertices>> _vertices_quads;
    };

//  [[plugin::builder]]
    class BUILDER : public Builder<Atlas> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        sp<Atlas> build(const Scope& args) override;

    private:
        SafeBuilder<Texture> _texture;
        std::vector<sp<Builder<AtlasImporter>>> _importers;
    };

private:
    sp<Texture> _texture;
    std::unordered_map<HashId, Item> _items;

    Traits _attachments;

    friend class BUILDER;

    DISALLOW_COPY_AND_ASSIGN(Atlas);
};

}
