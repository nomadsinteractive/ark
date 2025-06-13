#include "graphics/util/vec2i_type.h"

#include "core/base/bean_factory.h"
#include "core/types/implements.h"
#include "core/util/updatable_util.h"

namespace ark {

namespace {

class Vec2iImpl final : public Vec2i, Implements<Vec2iImpl, Vec2i> {
public:
    Vec2iImpl(sp<Integer> x)
        : _x(x), _y(std::move(x)) {
    }
    Vec2iImpl(sp<Integer> x, sp<Integer> y)
        : _x(std::move(x)), _y(std::move(y)) {
    }

    bool update(const uint64_t timestamp) override
    {
        return UpdatableUtil::update(timestamp, _x, _y);
    }

    V2i val() override
    {
        return {{_x->val(), _y->val()}};
    }

private:
    sp<Integer> _x;
    sp<Integer> _y;
};

}

sp<Vec2i> Vec2iType::create(const int32_t x, const int32_t y)
{
    return sp<Vec2i>::make<Vec2iImpl>(sp<Integer>::make<Integer::Const>(x), sp<Integer>::make<Integer::Const>(y));
}

sp<Vec2i> Vec2iType::create(sp<Integer> x, sp<Integer> y)
{
    ASSERT((x && y) || (x && !y));
    if(!y)
        return sp<Vec2i>::make<Vec2iImpl>(std::move(x));
    return sp<Vec2i>::make<Vec2iImpl>(std::move(x), std::move(y));
}

V2i Vec2iType::val(const sp<Vec2i>& self)
{
    self->update(Timestamp::now());
    return self->val();
}

size_t Vec2iType::len(const sp<Vec2i>& /*self*/)
{
    return 2;
}

Optional<int32_t> Vec2iType::getItem(const sp<Vec2i>& self, const ptrdiff_t index)
{
    return {self->val()[index]};
}

Vec2iType::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _x(factory.getBuilder<Integer>(manifest, "x")), _y(factory.getBuilder<Integer>(manifest, "y")), _value(factory.getBuilder<Vec2i>(manifest, "value"))
{
}

sp<Vec2i> Vec2iType::BUILDER::build(const Scope& args)
{
    if(_value)
        return _value->build(args);
    return create(_x.build(args), _y.build(args));
}

Vec2iType::DICTIONARY::DICTIONARY(BeanFactory& factory, const String& expr)
{
    factory.expand(expr, _x, _y);
}

sp<Vec2i> Vec2iType::DICTIONARY::build(const Scope& args)
{
    return create(_x->build(args), _y->build(args));
}

}
