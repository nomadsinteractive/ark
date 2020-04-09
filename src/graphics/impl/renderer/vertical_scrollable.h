#ifndef ARK_GRAPHICS_IMPL_RENDERER_VERTICAL_SCROLLABLE_H_
#define ARK_GRAPHICS_IMPL_RENDERER_VERTICAL_SCROLLABLE_H_

#include "core/base/api.h"
#include "core/inf/builder.h"
#include "core/types/implements.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/inf/block.h"
#include "graphics/inf/renderer.h"
#include "graphics/util/tiles.h"

#include "app/view/layout_hierarchy.h"

namespace ark {

class VerticalScrollable : public Renderer, public Block, public LayoutEventListener {
public:
    VerticalScrollable(const sp<RendererMaker>& tileMaker, const sp<Numeric>& scroller, const sp<Size>& size, int32_t tileHeight, uint32_t itemCount);

    virtual void render(RenderRequest& renderRequest, const V3& position) override;

    virtual const sp<Size>& size() override;

    virtual bool onEvent(const Event& event, float x, float y, bool ptin) override;

//  [[plugin::builder("vertical-scrollable")]]
    class BUILDER : public Builder<Renderer> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        virtual sp<Renderer> build(const Scope& args) override;

    private:
        sp<Builder<RendererMaker>> _renderer_maker;
        sp<Builder<Numeric>> _scroller;
        sp<Builder<Size>> _size;

        int32_t _tile_height;

        uint32_t _rows;
    };

private:
    void scrollTo(int32_t scrollPosition);

    void update();
    int32_t lower(int32_t pos) const;
    int32_t upper(int32_t pos) const;
    void ensureTile(RendererTile& tile, int32_t position);

    float toTileOffset(int32_t offset, int32_t gs);

private:
    sp<RendererMaker> _tile_maker;
    RollingList _tiles;
    sp<Numeric> _scroller;
    sp<Size> _size;

    const int32_t _height;
    const int32_t _tile_height;

    int32_t _scroll_position;
    int32_t _grid_position;

};

}

#endif
