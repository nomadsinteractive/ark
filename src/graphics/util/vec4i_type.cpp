#include "graphics/util/vec4i_type.h"

#include "core/base/bean_factory.h"
#include "core/types/implements.h"
#include "core/util/updatable_util.h"

namespace ark {

namespace {

class Vec4iImpl final : public Vec4i, Implements<Vec4iImpl, Vec4i> {
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
    self->update(Timestamp::now());
    return self->val();
}

size_t Vec4iType::len(const sp<Vec4i>& /*self*/)
{
    return 4;
}

Optional<int32_t> Vec4iType::getItem(const sp<Vec4i>& self, const ptrdiff_t index)
{
    return {self->val()[index]};
}

Vec4iType::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _x(factory.getBuilder<Integer>(manifest, "x")), _y(factory.getBuilder<Integer>(manifest, "y")), _z(factory.getBuilder<Integer>(manifest, "z")), _w(factory.getBuilder<Integer>(manifest, "w")),
      _value(factory.getBuilder<Vec4i>(manifest, "value"))
{
}

sp<Vec4i> Vec4iType::BUILDER::build(const Scope& args)
{
    if(_value)
        return _value->build(args);
    return create(_x.build(args), _y.build(args), _z.build(args), _w.build(args));
}

Vec4iType::DICTIONARY::DICTIONARY(BeanFactory& factory, const String& expr)
{
    factory.expand(expr, _x, _y, _z, _w);
}

sp<Vec4i> Vec4iType::DICTIONARY::build(const Scope& args)
{
    return create(_x->build(args), _y->build(args), _z->build(args), _w->build(args));
}

    
}
