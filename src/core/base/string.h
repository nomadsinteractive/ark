#pragma once

#include <functional>
#include <string>
#include <regex>
#include <vector>

#include "core/base/api.h"
#include "core/types/optional.h"

namespace ark {

class ARK_API String final {
public:
    typedef std::string::size_type size_type;

    String() = default;
    DEFAULT_COPY_AND_ASSIGN(String);

    String(const char* s);
    String(std::string str);
    ~String();

    String& operator =(const char* other);

    char at(size_type i) const;
    const char* c_str() const;
    const std::string& str() const;
    size_type length() const;
    size_type size() const;
    bool empty() const;
    HashId hash() const;
    size_type find(const String& str, size_type pos = 0) const;
    size_type find(char c, size_type pos = 0) const;
    size_type rfind(char c) const;

    const char& front() const;
    const char& back() const;

    String strip() const;
    String lstrip(char c) const;
    String rstrip(char c) const;
    String substr(size_type start) const;
    String substr(size_type start, size_type end) const;
    String replace(const String& text, const String& replacement) const;
    String replace(const std::regex& pattern, const std::function<String(Array<String>& matches)>& replacer) const;
    String toLower() const;

    StringView toStringView() const;

    std::pair<String, Optional<String>> cut(char sep) const;
    std::pair<Optional<String>, String> rcut(char sep) const;

    void insert(size_type pos, const String& str);

    std::vector<String> split(char delim, bool allowEmpty = false) const;
    void split(char delim, bool allowEmpty, const std::function<bool(const String&)>& traveller) const;

    array<String> match(const std::regex& pattern) const;
    bool search(const std::regex& pattern, const std::function<bool(const std::smatch& match)>& traveller) const;
    void search(const std::regex& pattern, const std::function<bool(const std::smatch& match)>& traveller1, const std::function<bool(const String& unmatch)>& traveller2) const;

    explicit operator bool() const;

    bool startsWith(const String& other) const;
    bool endsWith(const String& other) const;
    bool operator ==(const String& other) const;
    bool operator !=(const String& other) const;
    bool operator <(const String& other) const;
    bool operator >(const String& other) const;

    char operator[](std::size_t index) const;
    char& operator[](std::size_t index);

    static size_type npos;

    friend ARK_API String operator +(const String& lvalue, const String& rvalue);
    friend ARK_API String operator +(const String& lvalue, const char* rvalue);
    friend ARK_API String operator +(const char* lvalue, const String& rvalue);

    static const String& null();

private:
    std::string _str;

    friend class StringBuffer;
    friend class Strings;

};

}

template <> struct std::hash<ark::String> {
    size_t operator()(const ark::String& str) const {
        return str.hash();
    }
};
