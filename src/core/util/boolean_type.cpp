#include "core/util/boolean_type.h"

#include "core/base/bean_factory.h"
#include "core/impl/variable/variable_dyed.h"
#include "core/impl/variable/variable_expect.h"
#include "core/impl/variable/variable_op1.h"
#include "core/impl/variable/variable_op2.h"
#include "core/util/operators.h"

namespace ark {

namespace {

template<typename OP, typename T = Boolean> class OperationBuilder final : public Builder<Boolean> {
public:
    OperationBuilder(const sp<Builder<T>>& a1, const sp<Builder<T>>& a2)
        : _a1(a1), _a2(a2) {
    }

    sp<Boolean> build(const Scope& args) override {
        return sp<OP>::make(_a1->build(args), _a2->build(args));
    }

private:
    sp<Builder<T>> _a1;
    sp<Builder<T>> _a2;
};

class NegativeBooleanBuilder final : public Builder<Boolean> {
public:
    NegativeBooleanBuilder(sp<Builder<Boolean>> a1)
        : _a1(std::move(a1)) {
    }

    sp<Boolean> build(const Scope& args) override {
        return BooleanType::negate(_a1->build(args));
    }

private:
    sp<Builder<Boolean>> _a1;
};

class BooleanInTick final : public Boolean {
public:
    BooleanInTick(sp<Integer> tickStart, sp<Integer> tickEnd)
        : _tick_start(std::move(tickStart)), _tick_end(std::move(tickEnd)), _value() {
        ASSERT(_tick_start);
    }

    bool update(const uint32_t tick) override
    {
        _tick_start->update(tick);
        if(_tick_end)
            _tick_end->update(tick);

        bool value = tick >= static_cast<uint32_t>(_tick_start->val());
        if(_tick_end && value)
            value = tick < static_cast<uint32_t>(_tick_end->val());

        const bool dirty = _value != value;
        _value = value;
        return dirty;
    }

    bool val() override
    {
        return _value;
    }

private:
    sp<Integer> _tick_start;
    sp<Integer> _tick_end;
    bool _value;
};

}

sp<Boolean> BooleanType::create(const sp<Boolean>& value)
{
    return sp<Boolean>::make<BooleanWrapper>(value);
}

sp<Boolean> BooleanType::create(bool value)
{
    return sp<Boolean>::make<BooleanWrapper>(value);
}

sp<Boolean> BooleanType::__and__(sp<Boolean> self, sp<Boolean> rvalue)
{
    return sp<Boolean>::make<VariableOP2<sp<Boolean>, sp<Boolean>, Operators::And<bool>>>(std::move(self), std::move(rvalue));
}

sp<Boolean> BooleanType::__or__(sp<Boolean> self, sp<Boolean> rvalue)
{
    return sp<Boolean>::make<VariableOP2<sp<Boolean>, sp<Boolean>, Operators::Or<bool>>>(std::move(self), std::move(rvalue));
}

sp<Boolean> BooleanType::negate(sp<Boolean> self)
{
    return sp<Boolean>::make<VariableOP1<bool>>(Operators::Not<bool>(), std::move(self));
}

sp<Boolean> BooleanType::expect(sp<Boolean> self, sp<Boolean> expectation, sp<Future> future)
{
    return sp<Boolean>::make<VariableExpect<bool>>(std::move(self), std::move(expectation), std::move(future));
}

bool BooleanType::toBool(const sp<Boolean>& self)
{
    return update(self);
}

bool BooleanType::val(const sp<Boolean>& self)
{
    return self->val();
}

sp<Boolean> BooleanType::wrapped(const sp<Boolean>& self)
{
    const sp<BooleanWrapper>& ib = self.asInstance<BooleanWrapper>();
    DCHECK_WARN(ib, "Non-BooleanWrapper instance has no wrapped attribute. This should be an error unless you're inspecting it.");
    return ib ? ib->wrapped() : nullptr;
}

bool BooleanType::update(const sp<Boolean>& self)
{
    self->update(Timestamp::now());
    return self->val();
}

sp<Boolean> BooleanType::freeze(const sp<Boolean>& self)
{
    return sp<Boolean>::make<Boolean::Const>(update(self));
}

sp<Boolean> BooleanType::wrap(sp<Boolean> self)
{
    return sp<Boolean>::make<BooleanWrapper>(std::move(self));
}

void BooleanType::reset(const sp<BooleanWrapper>& self, sp<Boolean> value)
{
    if(value)
        self->set(std::move(value));
    else
        self->set(false);
}

void BooleanType::reset(const sp<BooleanWrapper>& self, const bool value)
{
    self->set(value);
}

sp<Boolean> BooleanType::dye(sp<Boolean> self, sp<Boolean> condition, String message)
{
#ifdef ARK_FLAG_PUBLISHING_BUILD
    WARN("Dyeing is a debugging technique, which should not be used in publish builds");
    return self;
#endif
    return sp<Boolean>::make<VariableDyed<bool>>(std::move(self), std::move(condition), std::move(message));
}

sp<Boolean> BooleanType::InTick(sp<Integer> tickStart, sp<Integer> tickEnd)
{
    return sp<Boolean>::make<BooleanInTick>(std::move(tickStart), std::move(tickEnd));
}

}
