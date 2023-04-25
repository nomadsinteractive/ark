#pragma once

#include "core/base/api.h"
#include "core/base/string.h"

namespace ark {

class ARK_API Attribute {
public:
    enum Type {
        TYPE_BYTE,
        TYPE_FLOAT,
        TYPE_INTEGER,
        TYPE_SHORT,
        TYPE_UBYTE,
        TYPE_USHORT,
        TYPE_COUNT
    };

    Attribute();
    Attribute(const String& name, Type type, const String& declareType, uint32_t length, bool normalized);
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(Attribute);

    const String& name() const;
    Type type() const;

    const String& declareType() const;

    uint32_t offset() const;
    void setOffset(uint32_t offset);

    uint32_t divisor() const;
    void setDivisor(uint32_t divisor);

    uint32_t length() const;

    uint32_t componentSize() const;
    uint32_t size() const;

    bool normalized() const;

private:
    String _name;
    Type _type;
    String _declare_type;

    uint32_t _offset;
    uint32_t _length;
    bool _normalized;

    uint32_t _divisor;
};

}
