#include "core/components/visibility.h"

#include "core/base/bean_factory.h"
#include "core/impl/variable/variable_wrapper.h"

namespace ark {

Visibility::Visibility(bool visible)
    : _visible(sp<BooleanWrapper>::make(visible))
{
}

Visibility::Visibility(const sp<Boolean>& visible)
    : _visible(sp<BooleanWrapper>::make(visible))
{
}

bool Visibility::val()
{
    return _visible->val();
}

bool Visibility::update(const uint32_t tick)
{
    return _visible->update(tick);
}

void Visibility::show()
{
    _visible->set(true);
}

void Visibility::hide()
{
    _visible->set(false);
}

void Visibility::reset(const bool visible)
{
    _visible->set(visible);
}

void Visibility::reset(sp<Boolean> visible)
{
    _visible->set(std::move(visible));
}

}
