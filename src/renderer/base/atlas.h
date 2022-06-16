#ifndef ARK_RENDERER_BASE_ATLAS_H_
#define ARK_RENDERER_BASE_ATLAS_H_

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/base/bean_factory.h"
#include "core/collection/by_index.h"
#include "core/collection/by_type.h"
#include "core/inf/builder.h"
#include "core/types/safe_ptr.h"
#include "core/types/shared_ptr.h"
#include "core/types/owned_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/base/rect.h"
#include "graphics/base/v2.h"

#include "renderer/forwarding.h"

namespace ark {

class ARK_API Atlas {
public:
    Atlas(sp<Texture> texture, bool allowDefaultItem = false);

    class ARK_API Item {
    public:
        Item();
        Item(uint16_t ux, uint16_t uy, uint16_t vx, uint16_t vy, const Rect& bounds, const V2& size, const V2& pivot);
        DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(Item);

        const Rect& bounds() const;
        const V2& size() const;
        const V2& pivot() const;

        Rect uv() const;

        uint16_t ux() const;
        uint16_t uy() const;
        uint16_t vx() const;
        uint16_t vy() const;

    private:
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
    bool has(int32_t c) const;
//  [[script::bindings::auto]]
    const V2& getOriginalSize(int32_t c) const;
//  [[script::bindings::auto]]
    const V2& getPivot(int32_t c) const;
//  [[script::bindings::auto]]
    Rect getItemUV(int32_t c) const;

//  [[script::bindings::auto]]
    sp<BitmapBundle> makeBitmapBundle() const;

//  [[script::bindings::auto]]
    void addImporter(const sp<AtlasImporter>& importer, const sp<Readable>& readable = nullptr);

    ByType& attachments();

    const std::unordered_map<int32_t, Item>& items() const;

    void add(int32_t id, uint32_t ux, uint32_t uy, uint32_t vx, uint32_t vy, const Rect& bounds, const V2& size, const V2& pivot);

    const Item& at(int32_t id) const;
    Rect getOriginalPosition(int32_t id) const;

    void clear();

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

        virtual sp<Atlas> build(const Scope& args) override;

    private:
        std::vector<document> _items;
        std::vector<sp<Builder<AtlasImporter>>> _importers;
        SafePtr<Builder<Texture>> _texture;
    };

private:
    Item makeItem(uint32_t ux, uint32_t uy, uint32_t vx, uint32_t vy, const Rect& bounds, const V2& size, const V2& pivot) const;

private:
    sp<Texture> _texture;

    float _width;
    float _height;

    bool _allow_default_item;
    Item _default_item;

    std::unordered_map<int32_t, Item> _items;

    ByType _attachments;

    friend class BUILDER;

    DISALLOW_COPY_AND_ASSIGN(Atlas);
};

}

#endif
