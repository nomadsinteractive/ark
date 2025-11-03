#pragma once

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/base/bean_factory.h"
#include "core/collection/traits.h"
#include "core/inf/builder.h"
#include "core/impl/builder/safe_builder.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/base/rect.h"
#include "graphics/base/v2.h"

#include "renderer/forwarding.h"

namespace ark {

class ARK_API Atlas {
public:
    Atlas(sp<Texture> texture);
//  [[script::bindings::auto]]
    Atlas(sp<Texture> texture, const String& src);

    struct UV {
        uint16_t _ux, _uy;
        uint16_t _vx, _vy;
    };

    struct Item {
        UV _uv;
        Rect _bounds;
        V2 _size;
        V2 _pivot;
    };

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

    const HashMap<HashId, Item>& items() const;
    HashMap<HashId, Item>& items();

    void add(int32_t id, uint32_t ux, uint32_t uy, uint32_t vx, uint32_t vy, const Rect& bounds, const V2& size, const V2& pivot);

    const Item& at(const NamedHash& resid) const;
    Rect getItemBounds(int32_t id) const;

    static UV toUV(uint32_t ux, uint32_t uy, uint32_t vx, uint32_t vy, uint32_t width, uint32_t height);
    static Item toItem(UV uv, const Rect& bounds, const V2& size, const V2& pivot);

    static uint16_t unnormalize(float v);
    static uint16_t unnormalize(uint32_t x, uint32_t s);

    class AttachmentNinePatch {
    public:
        const sp<Vertices>& ensureVerticesTriangleStrips(int32_t type, bool isLHS) const;
        const sp<Vertices>& ensureVerticesQuads(int32_t type, bool isLHS) const;

        void import(Atlas& atlas, const document& manifest);

        void add(int32_t type, uint32_t textureWidth, uint32_t textureHeight, const Rect& paddings, const Atlas& atlas);
        void addNinePatch(int32_t type, const Atlas& atlas, const String& s9);

    private:
        void addNinePatch(int32_t type, uint32_t textureWidth, uint32_t textureHeight, const Rect& ninePatch, const Rect& bounds);
        sp<Vertices> makeNinePatchVertices(uint32_t textureWidth, uint32_t textureHeight, const Rect& paddings, const Rect& bounds) const;

        struct NinePatchVertices {
            sp<Vertices> _triangle_strips_rhs;
            sp<Vertices> _quads_rhs;
            sp<Vertices> _triangle_strips_lhs;
            sp<Vertices> _quads_lhs;
        };

    private:
        Map<int32_t, NinePatchVertices> _nine_patch_vertices;
    };

//  [[plugin::builder]]
    class BUILDER final : public Builder<Atlas> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        sp<Atlas> build(const Scope& args) override;

    private:
        SafeBuilder<Texture> _texture;
        Vector<sp<Builder<AtlasImporter>>> _importers;
    };

private:
    sp<Texture> _texture;
    HashMap<HashId, Item> _items;

    Traits _attachments;

    friend class BUILDER;

    DISALLOW_COPY_AND_ASSIGN(Atlas);
};

}
