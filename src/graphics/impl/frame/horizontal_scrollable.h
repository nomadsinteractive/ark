#ifndef ARK_GRAPHICS_IMPL_FRAME_HORIZONTAL_SCROLLABLE_H_
#define ARK_GRAPHICS_IMPL_FRAME_HORIZONTAL_SCROLLABLE_H_

#include "core/base/api.h"
#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/inf/renderer.h"
#include "graphics/util/tiles.h"

namespace ark {

class HorizontalScrollable : public Renderer {
public:
    HorizontalScrollable(const sp<TileMaker>& tileMaker, const sp<Numeric>& scroller, int32_t width, int32_t tileWidth, uint32_t itemCount);

    virtual sp<RenderCommand> render(RenderRequest& renderRequest, float x, float y) override;

//  [[plugin::builder("horizontal-scrollable")]]
    class BUILDER : public Builder<Renderer> {
    public:
        BUILDER(BeanFactory& parent, const document& manifest);

        virtual sp<Renderer> build(const sp<Scope>& args) override;

    private:
        sp<Builder<TileMaker>> _tile_maker;
        sp<Builder<Numeric>> _scroller;

        sp<Builder<Numeric>> _width;
        sp<Builder<Numeric>> _tile_width;

        uint32_t _cols;
    };

private:
    void scrollTo(int32_t scrollPosition);

    int32_t lower(int32_t pos) const;
    int32_t upper(int32_t pos) const;
    void ensureTile(Tile<sp<Renderer>>& tile, int32_t position);

private:
    sp<TileMaker> _tile_maker;
    RollingList<Tile<sp<Renderer>>> _tiles;
    sp<Numeric> _scroller;

    const int32_t _width;
    const int32_t _tile_width;

    int32_t _scroll_position;
    int32_t _grid_position;

};

}

#endif
