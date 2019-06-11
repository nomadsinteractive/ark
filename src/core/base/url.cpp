#include "core/base/url.h"

namespace ark {

URL::URL(const String& url)
{
    parse(url.strip());
}

URL::URL(const String& protocol, const String& path)
    : _protocol(protocol), _path(lstrip(path, 0))
{
}

const String& URL::protocol() const
{
    return _protocol;
}

const String& URL::path() const
{
    return _path;
}

void URL::parse(const String& url)
{
    String::size_type p = url.find("://");
    if(p != String::npos)
    {
        _protocol = url.substr(0, p);
        _path = lstrip(url, p + 3);
    }
    else
        _path = lstrip(url, 0);
}

String URL::lstrip(const String& url, String::size_type offset) const
{
    String::size_type s = offset;
    while(s < url.length()) {
        if(url.find("./", s) == s)
            s += 2;
        else if(url.find("/", s) == s)
            s += 1;
        else
            break;
    }
    return s ? url.substr(s) : url;
}

}
