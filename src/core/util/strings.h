#pragma once

#include "core/base/api.h"
#include "core/base/string.h"
#include "core/base/string_buffer.h"
#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"
#include "core/util/string_convert.h"

namespace ark {

class ARK_API Strings {
public:
    static sp<Builder<String>> load(const String& resid);

    static String loadFromReadable(const sp<Readable>& readable);

    static String unwrap(const String& str, char open, char close);

    static void parentheses(const String& expr, String& lvalue, String& remaining);
    static size_t parentheses(const String& expr, size_t start, char open = '(', char close = ')');

    static bool parseArrayAndIndex(const String& expr, String& name, int32_t& index);

    static Map<String, String> parseProperties(const String& str, char delim = ';', char equal = ':');

    static String toUTF8(const std::wstring& text);
    static std::wstring fromUTF8(StringView text);

    static bool splitFunction(const String& expr, String& func, String& args);

    static String capitalizeFirst(const String& name);

    template<typename T> static T split(const String& str, char sep, char open = '(', char close = ')') {
        T r;
        int32_t depth = 0;
        StringBuffer sb;
        for(char c : str._str) {
            if(c == sep && depth == 0) {
                r.push_back(sb.str());
                sb.clear();
            }
            else {
                if(c == open)
                    ++depth;
                else if(c == close)
                    --depth;
                sb << c;
                DCHECK(depth >= 0, "Open close mismatch: %s", str.c_str());
            }
        }
        String tail = sb.str();
        if(tail)
            r.push_back(std::move(tail));
        return r;
    }

    template<typename T> static String toString(const T& value) {
        return StringConvert::repr<T>(value);
    }

    template<typename T> static T eval(const String& str) {
        return StringConvert::eval<T>(str);
    }

    static String sprintf(const char* format, ...);
    static String svprintf(const char* format, va_list args);

    template<typename T> static String join(const T* data, size_t offset, size_t length) {
        StringBuffer sb;
        for(size_t i = 0; i < length; i++) {
            if(i != 0)
                sb << ", ";
            sb << toString<T>(data[offset + i]);
        }
        return sb.str();
    }

    static String dumpMemory(const uint8_t* memory, size_t length);

    static String stripReference(const String& id);

    static bool isNumeric(const String& value);
    static bool isArgument(const String& value);
    static bool isVariableCharacter(char c, bool allowDash = true);
    static bool isVariableName(const String& name, bool allowDash = true);

//  [[plugin::builder::by-value]]
    class BUILDER : public Builder<String> {
    public:
        BUILDER(BeanFactory& factory, const String& value);

        sp<String> build(const Scope& args) override;

    private:
        sp<Builder<String>> _delegate;
    };

};

}
