#ifndef ARK_CORE_BASE_URL_H_
#define ARK_CORE_BASE_URL_H_

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

private:
    void parse(const String& url);

    String lstrip(const String& url, String::size_type offset) const;

private:
    String _protocol;
    String _path;

};

}

#endif
