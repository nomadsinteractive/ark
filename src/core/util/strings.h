#ifndef ARK_CORE_UTIL_STRINGS_H_
#define ARK_CORE_UTIL_STRINGS_H_

#include <map>

#include "core/base/api.h"
#include "core/base/string.h"
#include "core/base/string_buffer.h"
#include "core/impl/array/dynamic_array.h"
#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"
#include "core/util/conversions.h"

namespace ark {

class ARK_API Strings {
public:
    static sp<Builder<String>> load(const String& resid);
    static sp<Builder<String>> load(const document& manifest, const String& attr);
    static sp<Builder<String>> load(const document& manifest, const String& attr, const String& defvalue);

    static String loadFromReadable(const sp<Readable>& readable);

    static uint32_t hash(const String& text);

    static String unwrap(const String& str, char open, char close);
    static void cut(const String& str, String& left, String& right, char sep);
    static void rcut(const String& str, String& left, String& right, char sep);

    static void parentheses(const String& expr, String& lvalue, String& remaining);
    static size_t parentheses(const String& expr, size_t start, char open = '(', char close = ')');

    static bool parseNameValuePair(const String& expr, char equal, String& name, String& value);

    static std::map<String, String> parseProperties(const String& str, char delim = ';', char equal = ':');
    static const String& getProperty(const std::map<String, String>& properties, const String& key, const String& defValue = String::null());

    static String toUTF8(const std::wstring& text);
    static std::wstring fromUTF8(const String& text);

    static bool splitFunction(const String& expr, String& func, String& args);

    static String capitalFirst(const String& name);

    template<typename T> static array<T> toArray(const String& str, char open = '(', char close = ')') {
        const String value = Strings::unwrap(str.strip(), open, close);
        DASSERT(value);
        const std::vector<String> elems = value.split(',');
        const array<T> values = sp<DynamicArray<T>>::make(elems.size());
        for(size_t i = 0; i < elems.size(); i++)
            values->buf()[i] = parse<T>(elems[i]);
        return values;
    }

    template<typename T> static String toString(const T& value) {
        return Conversions::to<T, String>(value);
    }

    template<typename T> static T parse(const String& str) {
        return Conversions::to<String, T>(str);
    }

    template<typename T> static T getProperty(const std::map<String, String>& properties, const String& key) {
        return Strings::parse<T>(getProperty(properties, key));
    }

    static String sprintf(const char* format, ...);
    static String svprintf(const char* format, va_list args);

    template<typename T> static String join(const T* data, uint32_t offset, uint32_t length) {
        StringBuffer sb;
        for(uint32_t i = 0; i < length; i++) {
            if(i != 0)
                sb << ", ";
            sb << toString<T>(data[offset + i]);
        }
        return sb.str();
    }

    static String stripReference(const String& id);

    static bool isNumeric(const String& value);
    static bool isArgument(const String& value);
    static bool isVariableCharacter(char c, bool allowDash = true);
    static bool isVariableName(const String& name, bool allowDash = true);

//  [[plugin::builder::by-value]]
    class BUILDER : public Builder<String> {
    public:
        BUILDER(BeanFactory& factory, const String& value);

        virtual sp<String> build(const sp<Scope>& args) override;

    private:
        sp<Builder<String>> _delegate;
    };

};

}

#endif
