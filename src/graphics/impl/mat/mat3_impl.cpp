#include "graphics/impl/mat/mat3_impl.h"

#include "core/impl/variable/variable_wrapper.h"
#include "core/util/updatable_util.h"

#include "graphics/base/mat.h"

namespace ark {

namespace {

class TBNMat3 final : public Mat3 {
public:
    TBNMat3(const sp<Vec3>& t, const sp<Vec3>& b, const sp<Vec3>& n)
        : _t(t), _b(b), _n(n) {
    }

    M3 val() override {
        const V3 t = _t->val();
        const V3 b = _b->val();
        const V3 n = _n->val();
        float values[9] = {t.x(), b.x(), n.x(), t.y(), b.y(), n.y(), t.z(), b.z(), n.z()};
        return M3(values);
    }

    bool update(const uint64_t timestamp) override {
        return UpdatableUtil::update(timestamp, _t, _b, _n);
    }

private:
    sp<Vec3> _t;
    sp<Vec3> _b;
    sp<Vec3> _n;
};

}

Mat3Impl::Mat3Impl(const M3& mat) noexcept
    : _impl(sp<VariableWrapper<M3>>::make(mat))
{
}

Mat3Impl::Mat3Impl(const V3& t, const V3& b, const V3& n) noexcept
    : Mat3Impl()
{
    const float values[9] = {t.x(), b.x(), n.x(), t.y(), b.y(), n.y(), t.z(), b.z(), n.z()};
    _impl->set(M3(values));
}

Mat3Impl::Mat3Impl(const sp<Vec3>& t, const sp<Vec3>& b, const sp<Vec3>& n) noexcept
    : _impl(sp<VariableWrapper<M3>>::make(static_cast<sp<Mat3>>(sp<TBNMat3>::make(t, b, n))))
{
}

M3 Mat3Impl::val()
{
    return _impl->val();
}

bool Mat3Impl::update(uint64_t timestamp)
{
    return _impl->update(timestamp);
}

}
