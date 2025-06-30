#include "core/base/url.h"

#include "core/util/strings.h"

namespace ark {

namespace {

String lstrip(const String& url, const String::size_type offset)
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

String URL::toString() const
{
    if(_protocol)
        return Strings::sprintf("%s://%s", _protocol.c_str(), _path.c_str());
    return _path;
}

void URL::parse(const String& url)
{
    const String::size_type p = url.find("://");
    if(p != String::npos)
    {
        _protocol = url.substr(0, p);
        _path = lstrip(url, p + 3);
    }
    else
        _path = lstrip(url, 0);
}

}
