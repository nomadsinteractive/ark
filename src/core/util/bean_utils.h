#pragma once

#include "core/base/api.h"
#include "core/base/bean_factory.h"
#include "core/util/strings.h"

#include "core/forwarding.h"

namespace ark {

class [[deprecated]] BeanUtils {
public:

    template<typename... Args> static void split(BeanFactory& beanFactory, const String& str, sp<Builder<Args>>&... args) {
        const String value = Strings::unwrap(str.strip(), '(', ')');
        DCHECK(value, "Empty value being built");
        std::list<String> elems = Strings::split<std::list<String>>(value, ',');
        doSplit<Args...>(beanFactory, elems, args...);
    }

private:
    template<typename T, typename... Args> static void doSplit(BeanFactory& beanFactory, std::list<String>& elems, sp<Builder<T>>& builder, sp<Builder<Args>>&... args) {
        if(elems.empty())
            builder = getNullBuilder<T>();
        else {
            const String str = elems.front().strip();
            elems.pop_front();
            builder = beanFactory.getBuilder<T>(str);
        }
        if constexpr(sizeof...(args) > 0)
            doSplit<Args...>(beanFactory, elems, args...);
    }

    template<typename T> static sp<Builder<T>> getNullBuilder() {
        static const sp<Builder<T>> nb = sp<typename Builder<T>::Null>::make();
        return nb;
    }
};

}
