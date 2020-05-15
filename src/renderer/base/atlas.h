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
    Atlas(const sp<Texture>& texture, bool allowDefaultItem = false);

    class ARK_API Importer {
    public:
        virtual ~Importer() = default;

        virtual void import(Atlas& atlas, BeanFactory& factory, const document& manifest) = 0;
    };

    class ARK_API Item {
    public:
        Item();
        Item(uint16_t ux, uint16_t uy, uint16_t vx, uint16_t vy, const Rect& bounds, const V2& size);
        DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(Item);

        const Rect& bounds() const;
        const V2& size() const;

        uint16_t ux() const;
        uint16_t uy() const;
        uint16_t vx() const;
        uint16_t vy() const;

    private:
        uint16_t _ux, _uy;
        uint16_t _vx, _vy;
        Rect _bounds;
        V2 _size;
    };

    void load(BeanFactory& factory, const document& manifest, const Scope& args);

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
    Rect getItemUV(int32_t c) const;

    ByType& attachments();

    const op<ByIndex<Item>>& items() const;

    void add(int32_t id, uint32_t ux, uint32_t uy, uint32_t vx, uint32_t vy, const Rect& bounds, const V2& size, const V2& pivot);

    const Item& at(int32_t id) const;
    Rect getOriginalPosition(int32_t id) const;

    void clear();

    static uint16_t unnormalize(uint32_t x, uint32_t s);

//  [[plugin::builder]]
    class BUILDER : public Builder<Atlas> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        virtual sp<Atlas> build(const Scope& args) override;

    private:
        BeanFactory _factory;
        document _manifest;
        SafePtr<Builder<Texture>> _texture;
    };

private:
    Item makeItem(uint32_t ux, uint32_t uy, uint32_t vx, uint32_t vy, const Rect& bounds, const V2& size, const V2& pivot) const;

private:
    sp<Texture> _texture;

    op<ByIndex<Item>> _items;
    bool _allow_default_item;
    Item _default_item;

    ByType _attachments;

    friend class BUILDER;

    DISALLOW_COPY_AND_ASSIGN(Atlas);
};

}

#endif
