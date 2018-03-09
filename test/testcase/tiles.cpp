#include "core/types/shared_ptr.h"

#include "test/base/test_case.h"

#include "core/inf/variable.h"
#include "core/base/bean_factory.h"
#include "core/base/scope.h"

#include "graphics/base/render_request.h"
#include "graphics/inf/renderer.h"
#include "graphics/inf/tile_maker.h"
#include "graphics/impl/frame/vertical_scrollable.h"

namespace ark {
namespace unittest {

static float _render_position[4];
static int32_t _error_code = 0;
static int32_t _error_base = 1;
static const int32_t _tile_height = 50;

namespace {

class TileRendererImpl : public Renderer {
public:
    TileRendererImpl(int32_t tileX, int32_t tileY)
        : _tile_x(tileX), _tile_y(tileY) {
    }

    virtual void render(RenderRequest& /*pipeline*/, float /*x*/, float y) override {
        const int32_t idx = (_tile_y + _tile_height) / _tile_height;
        if(_render_position[idx] != y)
            _error_code = _error_base + idx;
    }

private:
    int32_t _tile_x, _tile_y;
};


class TileMakerImpl : public TileMaker {
public:
    virtual sp<Renderer> makeTile(int32_t x, int32_t y) override {
        printf("makeTile(%d, %d)\n", x, y);
        return sp<TileRendererImpl>::make(x, y);
    }
};

}

class TilesTestCase : public TestCase {
public:
    virtual int launch() {
        const sp<BeanFactory> beanFactory = this->getBeanFactory();
        const sp<TileMaker> tileMaker = sp<TileMakerImpl>::make();
        const sp<Scope> args = sp<Scope>::make();
        const sp<Numeric::Impl> scroller = sp<Numeric::Impl>::make(0.0f);
        args->put<Numeric>("scroller", scroller);
        args->put<TileMaker>("tile_maker", tileMaker);
        const sp<Renderer> scrollable = beanFactory->load<Renderer>("scrollable-001", args);
        int32_t errorCode, errorBase = 1;
        errorCode = checkScrollable(scrollable, NAN, 0.0f, 50.0f, NAN, errorBase);
        if(errorCode)
            return errorCode;
        scroller->set(20.0f);
        errorCode = checkScrollable(scrollable, NAN, -20.0f, 30.0f, 80.0f, errorBase);
        if(errorCode)
            return errorCode;
        scroller->set(50.0f);
        errorCode = checkScrollable(scrollable, NAN, NAN, 0.0f, 50.0f, errorBase);
        if(errorCode)
            return errorCode;
        scroller->set(-30.0f);
        errorCode = checkScrollable(scrollable, -20.0f, 30.0f, 80.0f, NAN, errorBase);
        if(errorCode)
            return errorCode;
        scroller->set(50.0f);
        errorCode = checkScrollable(scrollable, NAN, NAN, 0.0f, 50.0f, errorBase);
        if(errorCode)
            return errorCode;
        scroller->set(20.0f);
        errorCode = checkScrollable(scrollable, NAN, -20.0f, 30.0f, 80.0f, errorBase);
        if(errorCode)
            return errorCode;
        return 0;
    }

private:
    int32_t checkScrollable(const sp<Renderer>& scrollable, float p1, float p2, float p3, float p4, int32_t& errorBase) {
        _error_base = errorBase;
        errorBase += 4;
        _render_position[0] = p1;
        _render_position[1] = p2;
        _render_position[2] = p3;
        _render_position[3] = p4;
        RenderRequest pipeline(nullptr, nullptr, nullptr);
        scrollable->render(pipeline, 0.0f, 0.0f);
        return _error_code;
    }
};

}
}


ark::unittest::TestCase* tiles_create() {
    return new ark::unittest::TilesTestCase();
}
