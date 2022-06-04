#ifndef ARK_GRAPHICS_UTIL_TILES_H_
#define ARK_GRAPHICS_UTIL_TILES_H_

#include "core/forwarding.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/inf/tile_maker.h"

#include "app/inf/layout_event_listener.h"

namespace ark {

class RendererTile : public LayoutEventListener{
public:
    RendererTile();
    RendererTile(std::vector<sp<Renderer>> renderers, int32_t offset);

    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(RendererTile);

    virtual bool onEvent(const Event& event, float x, float y, bool ptin) override;

    explicit operator bool() const;

    int32_t offset() const;
    void setOffset(int32_t offset);

    void roll(int32_t offset);

    int32_t position() const;
    void setPosition(int32_t position);

    void setRenderer(std::vector<sp<Renderer>> renderers);
    void render(RenderRequest& renderRequest, const V3& position);

private:
    std::vector<sp<LayoutEventListener>> makeLayoutEventListeners() const;

private:
    std::vector<sp<Renderer>> _renderers;
    std::vector<sp<LayoutEventListener>> _layout_event_listeners;

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
