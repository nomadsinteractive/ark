#pragma once

#include <array>

#include "string_buffer.h"
#include "core/base/string.h"

namespace ark {

template<typename T> class BitSet {
public:
    typedef uint32_t convertable_type;

    constexpr BitSet()
        : _bits(0) {
    }
    constexpr BitSet(uint32_t bits)
        : _bits(bits) {
    }
    explicit operator bool() const {
        return static_cast<bool>(_bits);
    }

    void set(T bits, bool enabled = true) {
        if(enabled)
            _bits |= bits;
        else
            _bits &= ~bits;
    }

    bool has(T bits) const {
        return _bits & bits;
    }

    template<size_t N> static BitSet toBitSet(const String& value, const std::array<std::pair<const char*, T>, N>& bitNames) {
        convertable_type bitsets = 0;
        for(const String& i : value.split('|')) {
            convertable_type bitvalue = 0;
            for(const auto [k, v] : bitNames)
                if(i.strip() == k) {
                    bitvalue = v;
                    break;
                }
            if(!bitvalue) {
                StringBuffer sb;
                for(size_t j = 0; j < N; ++j) {
                    sb << bitNames.at(j).first << '(' << bitNames.at(j).second << ')';
                    if(j == N - 1)
                        sb << ", ";
                FATAL("Unknow value %s, possible values are [%s]", i.c_str(), sb.str().c_str());
                }
            }
            bitsets |= bitvalue;
        }
        return {bitsets};
    }

private:
    convertable_type _bits;
    static_assert(std::is_enum_v<T>, "Must be an enum type");
};

}
