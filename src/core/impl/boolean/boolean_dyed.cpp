#include "core/impl/boolean/boolean_dyed.h"

#include "core/ark.h"

namespace ark {

BooleanDyed::BooleanDyed(sp<Boolean> delegate, sp<Boolean> condition, String message)
    :  Wrapper(std::move(delegate)), _message(std::move(message))
{
}

bool BooleanDyed::update(uint64_t timestamp)
{
    _condition->update(timestamp);
    return _wrapped->update(timestamp);
}

bool BooleanDyed::val()
{
    TRACE(_condition->val(), _message.c_str());
    return _wrapped->val();
}

}
