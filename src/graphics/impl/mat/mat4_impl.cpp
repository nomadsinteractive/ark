#include "graphics/impl/mat/mat4_impl.h"

#include "core/impl/variable/variable_wrapper.h"
#include "core/util/updatable_util.h"

#include "graphics/base/mat.h"

namespace ark {

namespace {

class TBNMat4 final : public Mat4 {
public:
    TBNMat4(sp<Vec4> t, sp<Vec4> b, sp<Vec4> n, sp<Vec4> w)
        : _t(std::move(t)), _b(std::move(b)), _n(std::move(n)), _w(std::move(w)) {
    }

    M4 val() override {
        return toMatrix(_t->val(), _b->val(), _n->val(), _w->val());
    }

    bool update(const uint64_t timestamp) override {
        return UpdatableUtil::update(timestamp, _t, _b, _n, _w);
    }

    static M4 toMatrix(const V4 t, const V4 b, const V4 n, const V4 w) {
        const float values[16] = {t.x(), b.x(), n.x(), w.x(), t.y(), b.y(), n.y(), w.y(), t.z(), b.z(), n.z(), w.z(), t.w(), b.w(), n.w(), w.w()};
        return M4(values);
    }

private:
    sp<Vec4> _t;
    sp<Vec4> _b;
    sp<Vec4> _n;
    sp<Vec4> _w;
};

}

Mat4Impl::Mat4Impl(const M4& mat) noexcept
    : _impl(sp<VariableWrapper<M4>>::make(mat))
{
}

Mat4Impl::Mat4Impl(sp<Mat4> other) noexcept
    : _impl(sp<VariableWrapper<M4>>::make(std::move(other)))
{
}

Mat4Impl::Mat4Impl(const V4 t, const V4 b, const V4 n, const V4 w) noexcept
    : _impl(sp<VariableWrapper<M4>>::make(TBNMat4::toMatrix(t, b, n, w)))
{
}

Mat4Impl::Mat4Impl(sp<Vec4> t, sp<Vec4> b, sp<Vec4> n, sp<Vec4> w) noexcept
    : _impl(sp<VariableWrapper<M4>>::make(sp<Mat4>::make<TBNMat4>(std::move(t), std::move(b), std::move(n), std::move(w))))
{
}

M4 Mat4Impl::val()
{
    return _impl->val();
}

bool Mat4Impl::update(uint64_t timestamp)
{
    return _impl->update(timestamp);
}

void Mat4Impl::set(const M4& mat)
{
    _impl->set(mat);
}

void Mat4Impl::set(sp<Mat4> mat)
{
    _impl->set(std::move(mat));
}

}
