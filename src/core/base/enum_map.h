#pragma once

#include <map>
#include <optional>

#include "core/base/string.h"
#include "core/base/string_buffer.h"
#include "core/types/global.h"

namespace ark {

template<typename T> class [[deprecated]] EnumMap {
public:
    EnumMap() {
        initialize(_values);
    }

    std::optional<T> toEnum(const String& value) const {
        const auto iter = _values.find(value);
        return iter != _values.end() ? std::optional<T>(iter->second) : std::optional<T>();
    }

    T toEnumOrDefault(const String& value, T defValue) const {
        const auto iter = _values.find(value);
        return iter != _values.end() ? iter->second : defValue;
    }

    T ensureEnum(const String& value) const {
        const auto iter = _values.find(value);
        CHECK(iter != _values.end(), "Enum value \"%s\" not exist. Possible choices are: [%s", value.c_str(), joinKeys().c_str());
        return iter->second;
    }

    T toEnumCombo(const String& value) const {
        int32_t v = 0;
        for(const String& i : value.split('|')) {
            std::optional<T> opt = toEnum(i);
            CHECK(opt, "Unknow enum: \"%s\". Possible choices are: [%s]", value.c_str(), joinKeys().c_str());
            v |= opt.value();
        }
        return static_cast<T>(v);
    }

    static const EnumMap<T>& instance() {
        const Global<EnumMap<T>> inst;
        return inst;
    }

private:
    static void initialize(std::map<String, T>& enums);

    String joinKeys() const {
        StringBuffer sb;
        for(const auto& i : _values) {
            if(i != *_values.begin())
                sb << ", ";
            sb << "\"" << i.first << "\"";
        }
        return sb.str();
    }

private:
    std::map<String, T> _values;
};

}
