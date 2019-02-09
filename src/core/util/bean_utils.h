#ifndef ARK_CORE_UTIL_BEAN_UTILS_H_
#define ARK_CORE_UTIL_BEAN_UTILS_H_

#include <algorithm>

#include "core/base/api.h"
#include "core/base/bean_factory.h"
#include "core/util/documents.h"
#include "core/util/strings.h"

#include "core/forwarding.h"

namespace ark {

class ARK_API BeanUtils {
public:
    static int32_t toInteger(const sp<Builder<Numeric>>& t, const sp<Scope>& args, int32_t defValue = 0);

    static float toFloat(BeanFactory& args, const String& value, float defValue = 0);
    static float toFloat(const sp<Builder<Numeric>>& t, const sp<Scope>& args, float defValue = 0);

    template<typename... Args> static void split(BeanFactory& beanFactory, const document& manifest, const String& attr, sp<Builder<Args>>&... args) {
        split<Args...>(beanFactory, Documents::ensureAttribute(manifest, attr), args...);
    }

    template<typename... Args> static void split(BeanFactory& beanFactory, const String& str, sp<Builder<Args>>&... args) {
        const String value = Strings::unwrap(str.strip(), '(', ')');
        DCHECK(value, "Empty value being built");
        std::list<String> elems = Strings::split<std::list<String>>(value, ',');
        doSplit<Args...>(beanFactory, elems, args...);
    }

    template<typename T> static sp<T> as(const sp<Builder<Object>>& builder, const sp<Scope>& args) {
        const sp<Object> object = builder->build(args);
        DCHECK(object && object.is<T>(), "Illegal object");
        return object.as<T>();
    }

    template<typename T> static sp<T> as(BeanFactory& args, const String& str) {
        const sp<Object> object = args.ensure<Object>(str);
        DCHECK(object && object.is<T>(), "Illegal object");
        return object.as<T>();
    }

    template<typename T> static sp<T> as(BeanFactory& args, const document& doc, const String& attr) {
        return as<T>(args, Documents::ensureAttribute(doc, attr));
    }

private:
    template<typename T, typename... Args> static void doSplit(BeanFactory& beanFactory, std::list<String>& elems, sp<Builder<T>>& builder, sp<Builder<Args>>&... args) {
        if(elems.empty())
            builder = beanFactory.getNullBuilder<T>();
        else {
            const String str = elems.front();
            elems.pop_front();
            builder = beanFactory.getBuilder<T>(str);
            doSplit<Args...>(beanFactory, elems, args...);
        }
    }

    template<typename... Args> static void doSplit(BeanFactory& /*beanFactory*/, std::list<String>& /*elems*/) {
    }
};

}

#endif
