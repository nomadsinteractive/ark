#ifndef ARK_GRAPHICS_UTIL_TILES_H_
#define ARK_GRAPHICS_UTIL_TILES_H_

#include "core/forwarding.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/inf/tile_maker.h"

namespace ark {

class LayoutEventListener;

class RendererTile {
public:
    RendererTile();
    RendererTile(const sp<Renderer>& renderer, int32_t offset);

    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(RendererTile);

    explicit operator bool() const;

    int32_t offset() const;

    void setOffset(int32_t offset);

    void roll(int32_t offset);

    int32_t position() const;
    void setPosition(int32_t position);

    void setRenderer(sp<Renderer> renderer);
    void render(RenderRequest& renderRequest, const V3& position);

    const sp<LayoutEventListener>& layoutEventListener() const;

private:
    sp<Renderer> _renderer;
    sp<LayoutEventListener> _layout_event_listener;

    int32_t _offset;
    int32_t _position;

};

class RollingList {
public:
    RollingList(uint32_t itemCount);
    ~RollingList();

    void roll(int32_t offset);

    const RendererTile& operator[](uint32_t index) const;
    RendererTile& operator[](uint32_t index);

private:
    RendererTile* _items;
    uint32_t _item_count;
    uint32_t _head;
};

}

#endif
