#include "graphics/impl/mat/mat2_impl.h"

#include "core/impl/variable/variable_wrapper.h"
#include "core/util/variable_util.h"

#include "graphics/base/mat.h"

namespace ark {

namespace {

class TBMat2 : public Mat2 {
public:
    TBMat2(const sp<Vec2>& t, const sp<Vec2>& b)
        : _t(t), _b(b) {
    }

    virtual M2 val() override {
        const V2 t = _t->val();
        const V2 b = _b->val();
        const float values[4] = {t.x(), b.x(), t.y(), b.y()};
        return M2(values);
    }

    virtual bool update(uint64_t timestamp) override {
        return VariableUtil::update(timestamp, _t, _b);
    }

private:
    sp<Vec2> _t;
    sp<Vec2> _b;
};

}

Mat2Impl::Mat2Impl() noexcept
    : _impl(sp<VariableWrapper<M2>>::make(M2()))
{
    float values[4] = {1.0f, 0, 0, 1.0f};
    _impl->set(values);
}

Mat2Impl::Mat2Impl(const V2& t, const V2& b) noexcept
    : Mat2Impl()
{
    float values[4] = {t.x(), b.x(), t.y(), b.y()};
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

bool Mat2Impl::update(uint64_t timestamp)
{
    return _impl->update(timestamp);
}

void Mat2Impl::fix()
{
    _impl->fix();
}

}
