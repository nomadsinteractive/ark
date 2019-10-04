#ifndef ARK_GRAPHICS_IMPL_FRAME_SCORLLABLE_H_
#define ARK_GRAPHICS_IMPL_FRAME_SCORLLABLE_H_

#include "core/base/api.h"
#include "core/inf/builder.h"
#include "core/types/safe_ptr.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/inf/renderer.h"
#include "graphics/inf/block.h"
#include "graphics/inf/tile_maker.h"
#include "graphics/util/tiles.h"

namespace ark {

class ARK_API Scrollable : public Renderer, public Block {
public:
    struct ARK_API Params {
        Params(int32_t rowCount, int32_t colCount, int32_t rowIndex, int32_t colIndex, int32_t tileWidth, int32_t tileHeight);
        Params(const document& manifest);

        int32_t _row_count;
        int32_t _col_count;
        int32_t _row_index;
        int32_t _col_index;
        int32_t _tile_width;
        int32_t _tile_height;
    };

private:
    class RollingAdapter {
    public:
        RollingAdapter(const Params& params);
        RollingAdapter(const RollingAdapter& other);

        void setScrollXY(int32_t x, int32_t y);
        int32_t scrollX() const;
        int32_t scrollY() const;

        void roll(const RollingAdapter& rollingView, RendererMaker& tileMaker, int32_t tileWidth, int32_t tileHeight);

        const sp<Renderer>& getTile(RendererMaker& tileMaker, int32_t rowIndex, int32_t colIndex) const;
        void putTile(int32_t rowIndex, int32_t colIndex, const sp<Renderer>& tile);

    private:
        const Params& _params;
        array<sp<Renderer>> _tiles;
        int32_t _scroll_x;
        int32_t _scroll_y;
    };

public:
    Scrollable(const sp<Vec>& scroller, const sp<RendererMaker>& tileMaker, const sp<Size>& size, const Params& params);

    virtual void render(RenderRequest& renderRequest, const V3& position) override;

    virtual const SafePtr<Size>& size() override;

//  [[plugin::builder("scrollable")]]
    class BUILDER : public Builder<Renderer> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        virtual sp<Renderer> build(const Scope& args) override;

    private:
        sp<Builder<Vec>> _scroller;
        sp<Builder<RendererMaker>> _tile_maker;
        sp<Builder<Size>> _size;
        Params _params;
    };

private:
    void initialize();

    int32_t width() const;
    int32_t height() const;

    void update();
    void updateTask();

private:
    Params _params;
    RollingAdapter _rolling_view;
    sp<Vec> _scroller;
    sp<RendererMaker> _renderer_maker;
    SafePtr<Size> _size;
    int32_t _scroll_x;
    int32_t _scroll_y;


};

}

#endif
