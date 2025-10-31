#include "graphics/impl/mat/mat4_impl.h"

#include "core/impl/variable/variable_wrapper.h"
#include "core/util/updatable_util.h"

#include "graphics/base/mat.h"

namespace ark {

namespace {

M4 toMatrix(const V4 t, const V4 b, const V4 n, const V4 w) {
    const float values[16] = {t.x(), t.y(), t.z(), t.w(), b.x(), b.y(), b.z(), b.w(), n.x(), n.y(), n.z(), n.w(), w.x(), w.y(), w.z(), w.w()};
    return M4(values);
}

class TBNMat4 final : public Mat4 {
public:
    TBNMat4(sp<Vec4> t, sp<Vec4> b, sp<Vec4> n, sp<Vec4> w)
        : _t(std::move(t)), _b(std::move(b)), _n(std::move(n)), _w(std::move(w)) {
        update(Timestamp::now());
    }

    M4 val() override
    {
        return _matrix;
    }

    bool update(uint32_t tick) override
    {
        if(UpdatableUtil::update(tick, _t, _b, _n, _w))
        {
            _matrix = toMatrix(_t->val(), _b->val(), _n->val(), _w->val());
            return true;
        }
        return false;
    }

private:
    sp<Vec4> _t;
    sp<Vec4> _b;
    sp<Vec4> _n;
    sp<Vec4> _w;
    M4 _matrix;
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
    : _impl(sp<VariableWrapper<M4>>::make(toMatrix(t, b, n, w)))
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

bool Mat4Impl::update(uint32_t tick)
{
    return _impl->update(tick);
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
