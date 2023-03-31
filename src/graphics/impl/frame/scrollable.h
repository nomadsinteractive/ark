#pragma once

#include <vector>

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

class ARK_API Scrollable : public Block {
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
        RendererPool(float rendererWidth, float rendererHeight);

        const std::vector<Box>& cull(RendererMaker& rendererMaker, float x, float y, const Rect& viewport);

    private:
        void recycleOutOfFrustum(RendererMaker& rendererMaker, const Rect& viewport);

    private:
        typedef std::pair<float, float> RendererKey;

        int32_t _renderer_width;
        int32_t _renderer_height;
        std::map<RendererKey, std::vector<Box>> _renderers;
    };

public:
    Scrollable(sp<Vec3> scroller, sp<RendererMaker> rendererMaker, sp<Size> size, const Scrollable::Params& params);

    virtual const sp<Size>& size() override;

    const std::vector<Box>& cull(uint64_t timestamp);

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

private:
    int32_t width() const;
    int32_t height() const;

    void update(uint64_t timestamp);
    void updateTask();

private:
    Params _params;
    RendererPool _renderer_pool;
    sp<Vec3> _scroller;
    sp<RendererMaker> _renderer_maker;
    SafePtr<Size> _size;

    std::vector<Box> _culled_objects;
};

}
