#pragma once

#include "core/base/enum.h"
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

    void set(T bits, const bool enabled = true) {
        if(enabled)
            _bits |= toConvertableType(bits);
        else
            _bits &= ~toConvertableType(bits);
    }

    bool has(T bits) const {
        return _bits & toConvertableType(bits);
    }

    template<typename U> U toFlags(const std::function<U(T)>& converter, const size_t count) const {
        U flags = static_cast<U>(0);
        for(size_t i = 0; i < count; ++i)
            if(const T bits = static_cast<T>(SHIFT ? i : 1 << i); has(bits))
                flags = static_cast<U>(flags | converter(bits));
        return flags;
    }

    template<size_t N> using LookupTable = Enum::LookupTable<StringView, T, N>;
    template<size_t N> static BitSet toBitSet(const String& value, const LookupTable<N>& bitNames) {
        convertable_type bitsets = 0;
        for(const String& i : value.split('|'))
            bitsets |= Enum::lookup<T, N>(bitNames, i.strip());
        return {bitsets};
    }

private:
    constexpr static convertable_type toConvertableType(const convertable_type value) {
        if constexpr(SHIFT)
            return 1 << value;
        return value;
    }

private:
    convertable_type _bits;
    static_assert(std::is_enum_v<T>, "Must be an enum type");
};

}
