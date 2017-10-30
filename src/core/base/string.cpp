#include "core/base/string.h"

#include <algorithm>
#include <cctype>
#include <sstream>

#include "core/base/string_builder.h"
#include "core/inf/array.h"
#include "core/impl/array/dynamic_array.h"
#include "core/types/shared_ptr.h"

namespace ark {

String::size_type String::npos = std::string::npos;

String::String()
{
}

String::String(const char* s)
    : _str(s)
{
}

String::String(const std::string& str)
    : _str(str)
{
}

String::String(std::string&& str)
    : _str(std::move(str))
{
}

String::String(const String& other)
    : _str(other._str)
{
}

String::String(size_type length, char fill)
    : _str(length, fill)
{
}

String::String(String&& other)
    : _str(std::move(other._str))
{
}

String::~String()
{
}

const String& String::operator =(const String& other)
{
    _str = other._str;
    return *this;
}

const String& String::operator =(String&& other)
{
    _str = std::move(other._str);
    return *this;
}

const String& String::operator =(const char* other)
{
    _str = other;
    return *this;
}

char String::at(size_type i) const
{
    return _str.at(i);
}

const char* String::c_str() const
{
    return _str.c_str();
}

String::size_type String::length() const
{
    return _str.size();
}

bool String::empty() const
{
    return _str.empty();
}

String::size_type String::hash() const
{
    const char* str = _str.c_str();
    size_type h = 0;
    while(*str)
        h = h * 101 + *str++;
    return h;
}

String::size_type String::find(const String& str, size_type pos) const
{
    return _str.find(str._str, pos);
}

String::size_type String::find(char c, size_type pos) const
{
    return _str.find(c, pos);
}

String::size_type String::rfind(char c) const
{
    return _str.rfind(c);
}

String String::strip() const
{
    const char* front = _str.c_str();
    const char* back = front + _str.length() - 1;
    const char* wsFront = front;
    const char* wsBack = back;
    while(isspace(*wsFront))
        ++wsFront;
    while(wsBack >= wsFront && isspace(*wsBack))
        --wsBack;
    if(wsFront == front && wsBack == back)
        return *this;
    return wsBack < wsFront ? String() : std::string(wsFront, wsBack + 1);
}

String String::substr(size_type start) const
{
    return _str.substr(start);
}

String String::substr(size_type start, size_type end) const
{
    DCHECK(start <= end, "Illegal substr range(%d - %d)", start, end);
    return _str.substr(start, end - start);
}

String String::replace(const String& text, const String& replacement) const
{
    std::string s = _str;
    for(size_t pos = 0; ; pos += replacement.length())
    {
        pos = s.find(text._str, pos);
        if(pos == String::npos) break;

        s.erase(pos, text.length());
        s.insert(pos, replacement._str);
    }
    return s;
}

String String::replace(const std::regex& pattern, const std::function<String(Array<String>&)>& replacer) const
{
    sp<Array<String>> matches;
    std::string str = _str;
    std::smatch match;

    StringBuilder sb;

    while(std::regex_search(str, match, pattern))
    {
        if(!matches)
            matches = sp<DynamicArray<String>>::make(match.size());
        String* m = matches->array();
        for(size_type i = 0; i < match.size(); i ++)
            m[i] = match[i].str();
        sb << match.prefix().str();
        sb << replacer(*matches.get());
        str = match.suffix().str();
    }
    sb << str;
    return sb.str();
}

String String::toLower() const
{
    std::string s = _str;
    std::transform(s.begin(), s.end(), s.begin(), ::tolower);
    return s;
}

void String::insert(size_type pos, const String& str)
{
    _str.insert(pos, str._str);
}

std::vector<String> String::split(char delim, bool allowEmpty) const
{
    std::vector<String> elems;
    std::stringstream ss(_str);
    std::string item;
    while(std::getline(ss, item, delim))
    {
        const String s = String(item).strip();
        if(allowEmpty || !s.empty())
            elems.push_back(s);
    }
    return elems;
}

array<String> String::match(const std::regex& pattern) const
{
    std::smatch match;
    if(std::regex_match(_str, match, pattern))
    {
        const sp<Array<String>> matches = sp<DynamicArray<String>>::make(match.size());
        String* m = matches->array();
        for(size_type i = 0; i < match.size(); i ++)
            m[i] = match[i].str();
        return matches;
    }
    return nullptr;
}

void String::search(const std::regex& pattern, const std::function<bool(const std::smatch& match)>& traveller) const
{
    std::string str = _str;
    std::smatch match;
    while(std::regex_search(str, match, pattern))
    {
        if(!traveller(match))
            break;
        str = match.suffix().str();
    }
}

bool String::startsWith(const String& other) const
{
    return strncmp(_str.c_str(), other.c_str(), other.length()) == 0;
}

bool String::endsWith(const String& other) const
{
    return _str.length() >= other._str.length() && _str.compare(_str.length() - other._str.length(), other._str.length(), other._str) == 0;
}

String::operator bool() const
{
    return !_str.empty();
}

bool String::operator ==(const String& other) const
{
    return _str == other._str;
}

bool String::operator !=(const String& other) const
{
    return _str != other._str;
}

bool String::operator <(const String& other) const
{
    return _str < other._str;
}

bool String::operator >(const String& other) const
{
    return _str > other._str;
}

char String::operator[](size_t index) const
{
    return _str[index];
}

char& String::operator[](size_t index)
{
    return _str[index];
}

const String& String::null()
{
    static String inst;
    return inst;
}

String operator +(const String& lvalue, const char* rvalue)
{
    return lvalue._str + rvalue;
}

String operator +(const char* lvalue, const String& rvalue)
{
    return lvalue + rvalue._str;
}

String operator +(const String& lvalue, const String& rvalue)
{
    return lvalue._str + rvalue._str;
}

}
