#include "graphics/impl/mat/mat3_impl.h"

#include "core/impl/variable/variable_wrapper.h"
#include "core/util/updatable_util.h"

#include "graphics/base/mat.h"

namespace ark {

namespace {

M3 toMatrix(const V3 t, const V3 b, const V3 n) {
    const float values[9] = {t.x(), t.y(), t.z(), b.x(), b.y(), b.z(), n.x(), n.y(), n.z()};
    return M3(values);
}

class TBNMat3 final : public Mat3 {
public:
    TBNMat3(sp<Vec3> t, sp<Vec3> b, sp<Vec3> n)
        : _t(std::move(t)), _b(std::move(b)), _n(std::move(n)) {
        update(Timestamp::now());
    }

    M3 val() override {
        return _matrix;
    }

    bool update(uint32_t tick) override
    {
        if(UpdatableUtil::update(tick, _t, _b, _n))
        {
            _matrix = toMatrix(_t->val(), _b->val(), _n->val());
            return true;
        }
        return false;
    }

private:
    sp<Vec3> _t;
    sp<Vec3> _b;
    sp<Vec3> _n;
    M3 _matrix;
};

}

Mat3Impl::Mat3Impl(const M3& mat) noexcept
    : _impl(sp<VariableWrapper<M3>>::make(mat))
{
}

Mat3Impl::Mat3Impl(const V3 t, const V3 b, const V3 n) noexcept
    : Mat3Impl()
{
    _impl->set(toMatrix(t, b, n));
}

Mat3Impl::Mat3Impl(sp<Vec3> t, sp<Vec3> b, sp<Vec3> n) noexcept
    : _impl(sp<VariableWrapper<M3>>::make(sp<Mat3>::make<TBNMat3>(std::move(t), std::move(b), std::move(n))))
{
}

M3 Mat3Impl::val()
{
    return _impl->val();
}

bool Mat3Impl::update(uint32_t tick)
{
    return _impl->update(tick);
}

}
