#ifndef ARK_CORE_BASE_IDENTIFIER_H_
#define ARK_CORE_BASE_IDENTIFIER_H_

#include "core/base/api.h"
#include "core/base/string.h"

namespace ark {

class ARK_API Identifier {
public:
    enum Format {
        FORMAT_NONE,
        FORMAT_NAMESPACE,
        FORMAT_NAMESPACE_STRICT,
        FORMAT_URL,
        FORMAT_URL_STRICT
    };

    Identifier(const Identifier& other) = default;
    Identifier(Identifier&& other) = default;

    static Identifier parse(const String& s, Format format = FORMAT_NAMESPACE_STRICT);
    static Identifier parseRef(const String& s);

    const String& package() const;
    const String& arg() const;
    const String& ref() const;
    const String& val() const;

    String toString() const;

    bool isRef() const;
    bool isArg() const;
    bool isVal() const;

    enum IdType {
        ID_TYPE_VALUE = 0,
        ID_TYPE_REFERENCE = '@',
        ID_TYPE_ARGUMENT = '$'
    };

private:
    Identifier(IdType type, const String& package, const String& val);

    static bool parseAndVaildate(const String& s, String& package, String& val, Format format);

private:
    IdType _type;

    String _package;
    String _value;

};

}

#endif
