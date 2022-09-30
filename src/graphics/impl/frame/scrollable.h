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
        Params(int32_t rowCount, int32_t colCount, int32_t rowIndex, int32_t colIndex, int32_t rendererWidth, int32_t rendererHeight);

        int32_t _row_count;
        int32_t _col_count;
        int32_t _row_index;
        int32_t _col_index;
        int32_t _renderer_width;
        int32_t _renderer_height;
    };

private:
    class RendererPool {
    public:
        RendererPool(int32_t rendererWidth, int32_t rendererHeight);

        const std::vector<Box>& cull(RendererMaker& rendererMaker, int32_t x, int32_t y, const RectI& viewport);

    private:
        void recycleOutOfFrustum(RendererMaker& rendererMaker, const RectI& viewport);

    private:
        typedef std::pair<int32_t, int32_t> RendererKey;

        int32_t _renderer_width;
        int32_t _renderer_height;
        std::map<RendererKey, std::vector<Box>> _renderers;
    };

public:
    Scrollable(sp<Vec3> scroller, sp<RendererMaker> rendererMaker, sp<Size> size, const Scrollable::Params& params);

    virtual void render(RenderRequest& renderRequest, const V3& position) override;
    virtual const sp<Size>& size() override;

    void cull();

    const sp<Vec3>& scroller() const;
    const sp<RendererMaker>& rendererMaker() const;
    void setRendererMaker(const sp<RendererMaker>& rendererMaker);

//  [[plugin::builder]]
    class BUILDER_SCROLLABLE : public Builder<Scrollable> {
    public:
        BUILDER_SCROLLABLE(BeanFactory& factory, const document& manifest);

        virtual sp<Scrollable> build(const Scope& args) override;

    private:
        sp<Builder<Vec3>> _scroller;
        sp<Builder<RendererMaker>> _renderer_maker;
        sp<Builder<Size>> _size;

        int32_t _row_count;
        int32_t _col_count;
        sp<Builder<Numeric>> _renderer_width;
        sp<Builder<Numeric>> _renderer_height;
    };

//  [[plugin::builder("scrollable")]]
    class BUILDER_RENDERER : public Builder<Renderer> {
    public:
        BUILDER_RENDERER(BeanFactory& factory, const document& manifest);

        virtual sp<Renderer> build(const Scope& args) override;

    private:
        BUILDER_SCROLLABLE _impl;
    };

private:
    int32_t width() const;
    int32_t height() const;

    void update();
    void updateTask();

private:
    Params _params;
    RendererPool _renderer_pool;
    sp<Vec3> _scroller;
    sp<RendererMaker> _renderer_maker;
    SafePtr<Size> _size;
    int32_t _scroll_x;
    int32_t _scroll_y;
};

}

#endif
