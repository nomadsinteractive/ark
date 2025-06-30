#pragma once

#include "core/base/api.h"
#include "core/base/string.h"

namespace ark {

class ARK_API URL {
public:
    URL(const String& url);
    URL(const String& protocol, const String& path);
    DEFAULT_COPY_AND_ASSIGN(URL);

    const String& protocol() const;
    const String& path() const;

    String toString() const;

private:
    void parse(const String& url);

private:
    String _protocol;
    String _path;
};

}
