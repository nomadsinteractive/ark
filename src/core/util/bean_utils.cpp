#include "core/util/bean_utils.h"

#include "core/inf/variable.h"
#include "core/util/strings.h"


namespace ark {

int32_t BeanUtils::toInteger(const sp<Builder<Numeric>>& t, const sp<Scope>& args, int32_t defValue)
{
    if(!t)
        return defValue;
    const sp<Numeric> v = t->build(args);
    return v ? static_cast<int32_t>(v->val()) : defValue;
}

float BeanUtils::toFloat(BeanFactory& args, const String& value, float defValue)
{
    if(Strings::isNumeric(value))
        return Strings::parse<float>(value);

    const sp<Numeric> var = value.empty() ? nullptr : args.build<Numeric>(value);
    return var ? var->val() : defValue;
}

float BeanUtils::toFloat(const sp<Builder<Numeric>>& t, const sp<Scope>& args, float defValue)
{
    if(!t)
        return defValue;
    const sp<Numeric> v = t->build(args);
    return v ? v->val() : defValue;
}

}
