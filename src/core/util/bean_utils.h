#ifndef ARK_CORE_UTIL_BEAN_UTILS_H_
#define ARK_CORE_UTIL_BEAN_UTILS_H_

#include <algorithm>

#include "core/base/api.h"
#include "core/base/bean_factory.h"
#include "core/impl/array/dynamic_array.h"
#include "core/util/documents.h"

#include "core/forwarding.h"

namespace ark {

class ARK_API BeanUtils {
public:
    static int32_t toInteger(const sp<Builder<Numeric>>& t, const sp<Scope>& args, int32_t defValue = 0);

    static float toFloat(BeanFactory& args, const String& value, float defValue = 0);
    static float toFloat(const sp<Builder<Numeric>>& t, const sp<Scope>& args, float defValue = 0);

    template<typename T> static array<sp<Builder<T>>> split(BeanFactory& factory, const String& str) {
        const String value = Strings::unwrap(str.strip(), '(', ')');
        DCHECK(value, "Empty value being built");
        const std::vector<String> elems = value.split(',');
        const array<sp<Builder<T>>> builders = sp<DynamicArray<sp<Builder<T>>>>::make(elems.size());
        for(size_t i = 0; i < elems.size(); i++)
            builders->buf()[i] = factory.getBuilder<Numeric>(elems[i]);
        return builders;
    }

    template<typename... Args> static void parse(BeanFactory& beanFactory, const document& manifest, const String& attr, sp<Builder<Args>>&... args) {
        parse<Args...>(beanFactory, Documents::ensureAttribute(manifest, attr), args...);
    }

    template<typename... Args> static void parse(BeanFactory& beanFactory, const String& str, sp<Builder<Args>>&... args) {
        const String value = Strings::unwrap(str.strip(), '(', ')');
        DCHECK(value, "Empty value being built");
        const std::vector<String> splited = value.split(',');
        std::list<String> elems(splited.begin(), splited.end());
        _parse<Args...>(beanFactory, elems, args...);
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
    template<typename T, typename... Args> static void _parse(BeanFactory& beanFactory, std::list<String>& elems, sp<Builder<T>>& builder, sp<Builder<Args>>&... args) {
        if(!elems.empty()) {
            const String str = elems.front();
            elems.pop_front();
            builder = beanFactory.getBuilder<T>(str);
            _parse<Args...>(beanFactory, elems, args...);
        }
    }

    template<typename... Args> static void _parse(BeanFactory& /*beanFactory*/, std::list<String>& elems) {
    }
};

}

#endif
