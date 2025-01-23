#pragma once

#include "core/base/api.h"
#include "core/base/string.h"

namespace ark {

class ARK_API Identifier {
public:
    enum Type {
        ID_TYPE_AUTO = 0,
        ID_TYPE_VALUE = 1,
        ID_TYPE_VALUE_AND_TYPE = 2,
        ID_TYPE_REFERENCE = '@',
        ID_TYPE_ARGUMENT = '$',
        ID_TYPE_EXPRESSION = '{'
    };
    Identifier(Type type, String package, String val, String valueType = {});

    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(Identifier);

    static Identifier parse(const String& s, Type idType = ID_TYPE_AUTO, bool strictMode = true);
    static Identifier parseRef(const String& s, bool strictMode = true);

    Type type() const;

    const String& package() const;
    const String& arg() const;
    const String& ref() const;
    const String& val() const;
    const String& valType() const;

    String toString() const;

    bool isRef() const;
    bool isArg() const;
    bool isVal() const;

    Identifier withouPackage() const;

private:
    Type _type;
    String _package;
    String _value;
    String _value_type;
};

}
