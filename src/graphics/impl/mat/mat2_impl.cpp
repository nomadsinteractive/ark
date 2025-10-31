#include "graphics/impl/mat/mat2_impl.h"

#include "core/impl/variable/variable_wrapper.h"
#include "core/util/updatable_util.h"

#include "graphics/base/mat.h"

namespace ark {

namespace {

class TBMat2 final : public Mat2 {
public:
    TBMat2(const sp<Vec2>& t, const sp<Vec2>& b)
        : _t(t), _b(b) {
    }

    M2 val() override {
        const V2 t = _t->val();
        const V2 b = _b->val();
        const float values[4] = {t.x(), b.x(), t.y(), b.y()};
        return M2(values);
    }

    bool update(uint32_t tick) override {
        return UpdatableUtil::update(tick, _t, _b);
    }

private:
    sp<Vec2> _t;
    sp<Vec2> _b;
};

}

Mat2Impl::Mat2Impl(const M2& mat) noexcept
    : _impl(sp<VariableWrapper<M2>>::make(mat))
{
}

Mat2Impl::Mat2Impl(const V2& t, const V2& b) noexcept
    : Mat2Impl()
{
    const float values[4] = {t.x(), b.x(), t.y(), b.y()};
    _impl->set(M2(values));
}

Mat2Impl::Mat2Impl(const sp<Vec2>& t, const sp<Vec2>& b) noexcept
    : _impl(sp<VariableWrapper<M2>>::make(static_cast<sp<Mat2>>(sp<TBMat2>::make(t, b))))
{
}

M2 Mat2Impl::val()
{
    return _impl->val();
}

bool Mat2Impl::update(uint32_t tick)
{
    return _impl->update(tick);
}

}
