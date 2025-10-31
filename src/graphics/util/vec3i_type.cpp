#include "graphics/util/vec3i_type.h"

#include "core/base/bean_factory.h"
#include "core/types/implements.h"
#include "core/util/updatable_util.h"

namespace ark {

namespace {

class Vec3iImpl final : public Vec3i, Implements<Vec3iImpl, Vec3i> {
public:
    Vec3iImpl(sp<Integer> x)
        : _x(x), _y(x), _z(std::move(x)) {
    }
    Vec3iImpl(sp<Integer> x, sp<Integer> y, sp<Integer> z)
        : _x(std::move(x)), _y(std::move(y)), _z(std::move(z)) {
    }

    bool update(uint32_t tick) override
    {
        return UpdatableUtil::update(tick, _x, _y, _z);
    }

    V3i val() override
    {
        return {{_x->val(), _y->val(), _z->val()}};
    }

private:
    sp<Integer> _x;
    sp<Integer> _y;
    sp<Integer> _z;
};

}

sp<Vec3i> Vec3iType::create(const int32_t x, const int32_t y, const int32_t z)
{
    return sp<Vec3i>::make<Vec3iImpl>(sp<Integer>::make<Integer::Const>(x), sp<Integer>::make<Integer::Const>(y), sp<Integer>::make<Integer::Const>(z));
}

sp<Vec3i> Vec3iType::create(sp<Integer> x, sp<Integer> y, sp<Integer> z)
{
    ASSERT((x && y && z) || (x && !y && !z));
    if(!y)
        return sp<Vec3i>::make<Vec3iImpl>(std::move(x));
    return sp<Vec3i>::make<Vec3iImpl>(std::move(x), std::move(y), std::move(z));
}

V3i Vec3iType::val(const sp<Vec3i>& self)
{
    self->update(Timestamp::now());
    return self->val();
}

size_t Vec3iType::len(const sp<Vec3i>& /*self*/)
{
    return 3;
}

Optional<int32_t> Vec3iType::getItem(const sp<Vec3i>& self, const ptrdiff_t index)
{
    return {self->val()[index]};
}

Vec3iType::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _x(factory.getBuilder<Integer>(manifest, "x")), _y(factory.getBuilder<Integer>(manifest, "y")), _z(factory.getBuilder<Integer>(manifest, "z")),
      _value(factory.getBuilder<Vec3i>(manifest, "value"))
{
}

sp<Vec3i> Vec3iType::BUILDER::build(const Scope& args)
{
    if(_value)
        return _value->build(args);
    return create(_x.build(args), _y.build(args), _z.build(args));
}

Vec3iType::DICTIONARY::DICTIONARY(BeanFactory& factory, const String& expr)
{
    factory.expand(expr, _x, _y, _z);
}

sp<Vec3i> Vec3iType::DICTIONARY::build(const Scope& args)
{
    return create(_x->build(args), _y->build(args), _z->build(args));
}

    
}
