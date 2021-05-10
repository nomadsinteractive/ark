#ifndef ARK_CORE_IMPL_VARIABLE_INTERPOLATE_H_
#define ARK_CORE_IMPL_VARIABLE_INTERPOLATE_H_

#include "core/forwarding.h"
#include "core/inf/holder.h"
#include "core/inf/variable.h"
#include "core/types/implements.h"
#include "core/types/shared_ptr.h"
#include "core/util/holder_util.h"
#include "core/util/variable_util.h"

namespace ark {

template<typename T> class Interpolate : public Variable<T>, public Holder, Implements<Interpolate<T>, Variable<T>, Holder> {
public:
    Interpolate(sp<Variable<T>> a, sp<Variable<T>> b, sp<Variable<T>> v)
        : _a(std::move(a)), _b(std::move(b)), _v(std::move(v)), _a_freezed(_a->val()), _delta_freezed(_b->val() - _a_freezed), _val(_a_freezed + _delta_freezed * _v->val()) {
    }

    virtual T val() override {
        return _val;
    }

    virtual bool update(uint64_t timestamp) override {
        bool dirty = VariableUtil::update(timestamp, _a, _b);
        if(dirty) {
            _a_freezed = _a->val();
            _delta_freezed = _b->val() - _a_freezed;
        }
        if(_v->update(timestamp) || dirty) {
            _val = _a_freezed + _delta_freezed * _v->val();
            return true;
        }
        return false;
    }

    virtual void traverse(const Visitor& visitor) override {
        HolderUtil::visit(_a, visitor);
        HolderUtil::visit(_b, visitor);
        HolderUtil::visit(_v, visitor);
    }

private:
    sp<Variable<T>> _a;
    sp<Variable<T>> _b;
    sp<Variable<T>> _v;

    T _a_freezed;
    T _delta_freezed;
    T _val;
};

}

#endif
