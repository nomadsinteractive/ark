#include "app/impl/layout/frame_layout.h"

#include "graphics/traits/layout_param.h"

#include "app/util/layout_util.h"

namespace ark {

namespace {

class UpdatableFrameLayout : public Updatable {
public:
    bool update(uint64_t timestamp) override {
        return false;
    }
};

}

sp<Updatable> FrameLayout::inflate(Hierarchy hierarchy)
{
    return sp<UpdatableFrameLayout>::make();
}

sp<Layout> FrameLayout::BUILDER::build(const Scope& /*args*/)
{
    return sp<Layout>::make<FrameLayout>();
}

}
