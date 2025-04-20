#include "graphics/util/vec4i_type.h"

#include "core/util/updatable_util.h"

namespace ark {

namespace {

class Vec4iImpl final : public Vec4i {
public:
    Vec4iImpl(sp<Integer> x)
        : _x(x), _y(x), _z(x), _w(std::move(x)) {
    }
    Vec4iImpl(sp<Integer> x, sp<Integer> y, sp<Integer> z, sp<Integer> w)
        : _x(std::move(x)), _y(std::move(y)), _z(std::move(z)), _w(std::move(w)) {
    }

    bool update(const uint64_t timestamp) override
    {
        return UpdatableUtil::update(timestamp, _x, _y, _z, _w);
    }

    V4i val() override
    {
        return {{_x->val(), _y->val(), _z->val(), _w->val()}};
    }

private:
    sp<Integer> _x;
    sp<Integer> _y;
    sp<Integer> _z;
    sp<Integer> _w;
};

}

sp<Vec4i> Vec4iType::create(const int32_t x, const int32_t y, const int32_t z, const int32_t w)
{
    return sp<Vec4i>::make<Vec4iImpl>(sp<Integer>::make<Integer::Const>(x), sp<Integer>::make<Integer::Const>(y), sp<Integer>::make<Integer::Const>(z), sp<Integer>::make<Integer::Const>(w));
}

sp<Vec4i> Vec4iType::create(sp<Integer> x, sp<Integer> y, sp<Integer> z, sp<Integer> w)
{
    ASSERT((x && y && z && w) || (x && !y && !z && !w));
    if(!y)
        return sp<Vec4i>::make<Vec4iImpl>(std::move(x));
    return sp<Vec4i>::make<Vec4iImpl>(std::move(x), std::move(y), std::move(z), std::move(w));
}

V4i Vec4iType::val(const sp<Vec4i>& self)
{
    self->update(0);
    return self->val();
}

size_t Vec4iType::len(const sp<Vec4i>& /*self*/)
{
    return 4;
}

Optional<int32_t> Vec4iType::getItem(const sp<Vec4i>& self, ptrdiff_t index)
{
    return {self->val()[index]};
}

}
