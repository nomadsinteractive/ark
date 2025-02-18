#pragma once

#include <array>

#include "string_buffer.h"
#include "core/base/string.h"

namespace ark {

template<typename T, bool SHIFT = false> class BitSet {
public:
    typedef uint32_t convertable_type;

    constexpr BitSet()
        : _bits(0) {
    }
    constexpr BitSet(const convertable_type bits)
        : _bits(bits) {
    }
    DEFAULT_COPY_AND_ASSIGN(BitSet);

    explicit operator bool() const {
        return static_cast<bool>(_bits);
    }

    bool operator ==(T other) const {
        return _bits == toConvertableType(other);
    }
    bool operator ==(const BitSet other) const {
        return _bits == other.bits();
    }
    bool operator !=(T other) const {
        return _bits != toConvertableType(other);
    }
    bool operator !=(const BitSet other) const {
        return _bits != other.bits();
    }
    BitSet operator |(T other) const {
        return BitSet(_bits | other);
    }
    BitSet operator |(const BitSet other) const {
        return BitSet(_bits | other.bits());
    }
    BitSet operator &(T other) const {
        return BitSet(_bits & other);
    }
    BitSet operator &(const BitSet other) const {
        return BitSet(_bits & other.bits());
    }

    convertable_type bits() const {
        return _bits;
    }

    void set(T bits, bool enabled = true) {
        if(enabled)
            _bits |= toConvertableType(bits);
        else
            _bits &= ~toConvertableType(bits);
    }

    bool has(T bits) const {
        return _bits & toConvertableType(bits);
    }

    template<typename U> U toFlags(const std::function<U(T)>& converter, size_t count) const {
        U flags = static_cast<U>(0);
        for(size_t i = 0; i < count; ++i)
            if(const T bits = static_cast<T>(SHIFT ? i : 1 << i); has(bits))
                flags = static_cast<U>(flags | converter(bits));
        return flags;
    }

    template<typename... Args> static constexpr BitSet toBitSet(Args&&... bits) {
        if constexpr(sizeof...(bits) == 0)
            return {};

        return {toConvertableType(std::forward<Args>(bits)...)};
    }

    template<size_t N> static BitSet toBitSet(const String& value, const std::array<std::pair<const char*, T>, N>& bitNames) {
        convertable_type bitsets = 0;
        for(const String& i : value.split('|')) {
            convertable_type bitvalue = std::numeric_limits<convertable_type>::max();
            for(const auto [k, v] : bitNames)
                if(i.strip() == k) {
                    bitvalue = toConvertableType(v);
                    break;
                }
            if(bitvalue == std::numeric_limits<convertable_type>::max()) {
                StringBuffer sb;
                for(size_t j = 0; j < N; ++j) {
                    sb << bitNames.at(j).first << '(' << bitNames.at(j).second << ')';
                    if(j == N - 1)
                        sb << ", ";
                }
                FATAL("Unknow value %s, possible values are [%s]", i.c_str(), sb.str().c_str());
            }
            bitsets |= bitvalue;
        }
        return {bitsets};
    }

private:
    template<typename... Args> constexpr static convertable_type toConvertableType(convertable_type value, Args&&... vars) {
        if constexpr(sizeof...(vars) > 0) {
            if constexpr(SHIFT)
                return 1 << value | toConvertableType(std::forward<Args>(vars)...);
            return value | toConvertableType(std::forward<Args>(vars)...);
        }

        if constexpr(SHIFT)
            return 1 << value;
        return value;
    }

private:
    convertable_type _bits;
    static_assert(std::is_enum_v<T>, "Must be an enum type");
};

}
