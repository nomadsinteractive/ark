#include "core/util/dictionaries.h"

#include "core/base/bean_factory.h"
#include "core/inf/variable.h"
#include "core/util/strings.h"

namespace ark {

template<> ARK_API float Dictionaries::get<float>(BeanFactory& beanFactory, const String& value, const sp<Scope>& args)
{
    if(Strings::isNumeric(value))
        return Strings::parse<float>(value);

    const sp<Numeric> var = value.empty() ? nullptr : beanFactory.build<Numeric>(value, args);
    return var ? var->val() : 0.0f;
}

template<> ARK_API String Dictionaries::get<String>(BeanFactory& /*beanFactory*/, const String& value, const sp<Scope>& /*args*/)
{
    return value;
}

}
