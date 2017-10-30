#ifndef ARK_GRAPHICS_IMPL_FRAME_VERTICAL_SCROLLABLE_H_
#define ARK_GRAPHICS_IMPL_FRAME_VERTICAL_SCROLLABLE_H_

#include "core/base/api.h"
#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/inf/renderer.h"
#include "graphics/util/tiles.h"

namespace ark {

class VerticalScrollable : public Renderer {
public:
    VerticalScrollable(const sp<TileMaker>& tileMaker, const sp<Numeric>& scroller, int32_t height, int32_t tileHeight, uint32_t itemCount);

    virtual void render(RenderCommandPipeline& pipeline, float x, float y) override;

//  [[plugin::builder("vertical-scrollable")]]
    class BUILDER : public Builder<Renderer> {
    public:
        BUILDER(BeanFactory& parent, const document& doc);

        virtual sp<Renderer> build(const sp<Scope>& args) override;

    private:
        sp<Builder<TileMaker>> _tile_maker;
        sp<Builder<Numeric>> _scroller;

        int32_t _height;
        int32_t _tile_height;

        uint32_t _rows;
    };

private:
    void scrollTo(int32_t scrollPosition);

    void update();
    int32_t lower(int32_t pos) const;
    int32_t upper(int32_t pos) const;
    void ensureTile(Tile<sp<Renderer>>& tile, int32_t position);

private:
    sp<TileMaker> _tile_maker;
    RollingList<Tile<sp<Renderer>>> _tiles;
    sp<Numeric> _scroller;

    const int32_t _height;
    const int32_t _tile_height;

    int32_t _scroll_position;
    int32_t _grid_position;

};

}

#endif
