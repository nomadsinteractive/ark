#include "graphics/impl/mat/mat4_impl.h"

#include "core/impl/variable/variable_wrapper.h"
#include "core/util/variable_util.h"

#include "graphics/base/mat.h"

namespace ark {

namespace {

class TBNMat4 : public Mat4 {
public:
    TBNMat4(const sp<Vec4>& t, const sp<Vec4>& b, const sp<Vec4>& n, const sp<Vec4>& w)
        : _t(t), _b(b), _n(n), _w(w) {
    }

    virtual M4 val() override {
        return toMatrix(_t->val(), _b->val(), _n->val(), _w->val());
    }

    virtual bool update(uint64_t timestamp) override {
        return VariableUtil::update(timestamp, _t, _b, _n, _w);
    }

    static M4 toMatrix(const V4& t, const V4& b, const V4& n, const V4& w) {
        float values[16] = {t.x(), b.x(), n.x(), w.x(), t.y(), b.y(), n.y(), w.y(), t.z(), b.z(), n.z(), w.z(), t.w(), b.w(), n.w(), w.w()};
        return M4(values);
    }

private:
    sp<Vec4> _t;
    sp<Vec4> _b;
    sp<Vec4> _n;
    sp<Vec4> _w;
};

}

Mat4Impl::Mat4Impl() noexcept
    : _impl(sp<VariableWrapper<M4>>::make(M4::identity()))
{
}

Mat4Impl::Mat4Impl(const V4& t, const V4& b, const V4& n, const V4& w) noexcept
    : _impl(sp<VariableWrapper<M4>>::make(TBNMat4::toMatrix(t, b, n, w)))
{
}

Mat4Impl::Mat4Impl(const sp<Vec4>& t, const sp<Vec4>& b, const sp<Vec4>& n, const sp<Vec4>& w) noexcept
    : _impl(sp<VariableWrapper<M4>>::make(sp<Mat4>::make<TBNMat4>(t, b, n, w)))
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

void Mat4Impl::fix()
{
    _impl->fix();
}

}
