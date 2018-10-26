#ifndef ARK_RENDERER_BASE_ATLAS_H_
#define ARK_RENDERER_BASE_ATLAS_H_

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/base/bean_factory.h"
#include "core/collection/by_index.h"
#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/base/rect.h"

#include "renderer/forwarding.h"

namespace ark {

class ARK_API Atlas {
public:
    Atlas(const sp<GLTexture>& texture, bool allowDefaultItem = false);

    class ARK_API Importer {
    public:
        virtual ~Importer() = default;

        virtual void import(Atlas& atlas, const ResourceLoaderContext& resourceLoaderContext, const document& manifest) = 0;
    };

    class ARK_API Item {
    public:
        Item();
        Item(uint16_t left, uint16_t top, uint16_t right, uint16_t bottom, float width, float height, float pivotX = 0, float pivotY = 0);
        DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(Item);

        const sp<Size>& size() const;

        uint16_t left() const;
        uint16_t top() const;
        uint16_t right() const;
        uint16_t bottom() const;

        float width() const;
        float height() const;

        float pivotX() const;
        float pivotY() const;

    private:
        uint16_t _left, _top;
        uint16_t _right, _bottom;
        sp<Size> _size;
        float _pivot_x, _pivot_y;
    };

    const sp<GLTexture>& texture() const;
    uint32_t width() const;
    uint32_t height() const;

    uint16_t halfPixelX() const;
    uint16_t halfPixelY() const;

    bool has(int32_t c) const;
    void add(int32_t id, uint32_t left, uint32_t top, uint32_t right, uint32_t bottom, float pivotX = 0, float pivotY = 0);

    const Item& at(int32_t id) const;
    void getOriginalPosition(int32_t id, Rect& position) const;

    void clear();

    static uint16_t unnormalize(uint32_t x, uint32_t s);

//  [[plugin::resource-loader]]
    class BUILDER : public Builder<Atlas> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext);

        virtual sp<Atlas> build(const sp<Scope>& args) override;

    private:
        BeanFactory _factory;
        document _manifest;
        sp<Builder<Atlas>> _atlas;
        sp<Builder<GLTexture>> _texture;
        sp<ResourceLoaderContext> _resource_loader_context;
    };
private:
    Item makeItem(uint32_t left, uint32_t top, uint32_t right, uint32_t bottom, float pivotX, float pivotY) const;

private:
    sp<GLTexture> _texture;
    uint16_t _half_pixel_x, _half_pixel_y;

    op<ByIndex<Item>> _atlas;
    bool _allow_default_item;
    Item _default_item;

    friend class BUILDER;

    DISALLOW_COPY_AND_ASSIGN(Atlas);
};

}

#endif
