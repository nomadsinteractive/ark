#include "core/util/strings.h"

#include <algorithm>
#include <cstdarg>
#include <cctype>
#include <cstdio>
#include <string>
#include <regex>

#include "core/base/string_table.h"
#include "core/base/scope.h"
#include "core/inf/array.h"
#include "core/inf/builder.h"
#include "core/inf/readable.h"
#include "core/inf/variable.h"
#include "core/types/global.h"
#include "core/types/shared_ptr.h"
#include "core/util/documents.h"
#include "core/base/identifier.h"

namespace ark {

namespace {

Vector<sp<IBuilder<String>>> regexp_split(const std::string& s, const std::regex& pattern, const std::function<sp<IBuilder<String>>(const std::smatch&)>& replacer)
{
    std::smatch match;
    std::string str = s;
    Vector<sp<IBuilder<String>>> items;

    while(std::regex_search(str, match, pattern))
    {
        if(!match.prefix().str().empty())
            items.push_back(sp<IBuilder<String>::Prebuilt>::make(match.prefix().str()));
        items.push_back(replacer(match));
        str = match.suffix().str();
    }
    if(!str.empty())
        items.push_back(sp<IBuilder<String>::Prebuilt>::make(String(std::move(str))));
    return items;
}

class StringBuilderImpl1 final : public IBuilder<String> {
public:
    StringBuilderImpl1(const String& package, const String& resid)
        : _package(package), _resid(resid) {
    }

    String build(const Scope& /*args*/) override {
        const Global<StringTable> stringTable;
        return std::move(stringTable->getString(_package, _resid, true).value());
    }

private:
    String _package;
    String _resid;
};

class StringBuilderImpl2 final : public IBuilder<String> {
public:
    StringBuilderImpl2(const String& name)
        : _name(name) {
    }

    String build(const Scope& args) override {
        Optional<Box> optValue = args.getObject(_name);
        const sp<String> value = optValue ? optValue->as<String>() : nullptr;
        CHECK(value, "Cannot get argument \"%s\"", _name.c_str());
        return std::move(*value);
    }

private:
    String _name;
};

class StringBuilderImpl3 final : public IBuilder<String> {
public:
    StringBuilderImpl3(const String& value)
    {
        static const std::regex VAR_PATTERN("([$@])\\{?([\\w.:\\-?&/]+)\\}?");
        _builders = regexp_split(value.c_str(), VAR_PATTERN, [](const std::smatch& match) -> sp<IBuilder<String>> {
            const String& p = match[1].str();
            const String& s = match[2].str();
            if(p == "$")
                return sp<IBuilder<String>>::make<StringBuilderImpl2>(s);
            const Identifier id = Identifier::parseRef(s, false);
            return sp<IBuilder<String>>::make<StringBuilderImpl1>(id.package(), id.ref());
        });
    }

    String build(const Scope& args) override {
        StringBuffer sb;
        for(const sp<IBuilder<String>>& i : _builders) {
            const String v = i->build(args);
            sb << v.c_str();
        }
        return sb.str();
    }

private:
    Vector<sp<IBuilder<String>>> _builders;
};

}

sp<IBuilder<String>> Strings::load(const String& resid)
{
    if(!resid || resid.find('{') != String::npos)
        return sp<IBuilder<String>>::make<StringBuilderImpl3>(resid);

    const Identifier id = Identifier::parse(resid, Identifier::ID_TYPE_AUTO, false);
    if(id.isRef())
        return sp<IBuilder<String>>::make<StringBuilderImpl1>(id.package(), id.ref());
    if(id.isArg())
        return sp<IBuilder<String>>::make<StringBuilderImpl2>(id.arg());
    return sp<IBuilder<String>>::make<StringBuilderImpl3>(resid);
}

String Strings::loadFromReadable(Readable& readable)
{
    std::stringstream sb;
    uint32_t len;
    char buffer[4096];
    while((len = readable.read(buffer, sizeof(buffer))) != 0)
        sb.write(buffer, len);
    return {sb.str().c_str()};
}

String Strings::unwrap(const String& str, const char open, const char close)
{
    if(!str.empty() && str.at(0) == open && str.at(str.length() - 1) == close)
        return str.substr(1, str.length() - 1);
    return str;
}

void Strings::parentheses(const String& expr, String& lvalue, String& remaining)
{
    const size_t pos = parentheses(expr, 0);
    lvalue = expr.substr(1, pos);
    remaining = expr.substr(pos + 1).strip();
}

size_t Strings::parentheses(const String& expr, size_t start, char open, char close)
{
    DCHECK(expr.length() > start, "Illegal expression: unexpected end");
    CHECK(expr.at(start) == open, "Illegal expression: \"%s\", parentheses unmatch", expr.c_str());
    const size_t size = expr.length();
    int32_t count = 1;
    for(size_t i = start + 1; i < size; i++)
    {
        char c = expr.at(i);
        if(c == open)
            count++;
        else if(c == close && --count == 0)
            return i;
    }
    FATAL("Illegal expression: \"%s\", parentheses unmatch", expr.c_str());
    return 0;
}

bool Strings::parseArrayAndIndex(const String& expr, String& name, int32_t& index)
{
    const auto s1 = expr.find('[');
    if(s1 == String::npos)
        return false;
    const auto s2 = expr.find(']');
    if(s2 == String::npos)
        return false;

    name = expr.substr(0, s1 - 1);
    index = eval<int32_t>(expr.substr(s1 + 1, s2));
    return true;
}

Map<String, String> Strings::parseProperties(const String& str, char delim, char equal)
{
    Map<String, String> properties;
    Vector<String> elems = str.split(delim);
    for(const String& i : elems)
    {
        const auto [key, value] = i.cut(equal);
        properties[key] = value ? value.value() : "";
    }
    return properties;
}

typedef unsigned char byte_t;

// Reference: http://en.wikipedia.org/wiki/Utf8
static const byte_t cUtf8Limits[] =
{
    0xC0, // Start of a 2-byte sequence
    0xE0, // Start of a 3-byte sequence
    0xF0, // Start of a 4-byte sequence
    0xF8, // Start of a 5-byte sequence
    0xFC, // Start of a 6-byte sequence
    0xFE // Invalid: not defined by original UTF-8 specification
};

/*! Usually it is a 2 steps process to convert the string, invoke utf8ToUtf16() with
 dest equals to null so that it gives you destLen (not including null terminator),
 then allocate the destination with that amount of memory and call utf8ToUtf16() once
 again to perform the actual conversion. You can skip the first call if you sure
 the destination buffer is large enough to store the data.

 \note Here we assum sizeof(wchar_t) == 2
 \ref Modify from 7zip LZMA sdk
 */
bool utf8ToUtf16(wchar_t* dest, size_t& destLen, const char* src, size_t maxSrcLen)
{
    size_t destPos = 0, srcPos = 0;

    while(true)
    {
        byte_t c; // Note that byte_t should be unsigned
        size_t numAdds;

        if(srcPos == maxSrcLen || src[srcPos] == '\0')
        {
            if(dest && destLen != destPos)
            {
                DFATAL("The provided destLen should equals to what we calculated here");
                return false;
            }

            destLen = destPos;
            return true;
        }

        c = src[srcPos++];

        if(c < 0x80)   // 0-127, US-ASCII (single byte)
        {
            if(dest)
                dest[destPos] = static_cast<wchar_t>(c);
            ++destPos;
            continue;
        }

        if(c < 0xC0) // The first octet for each code point should within 0-191
            break;

        for(numAdds = 1; numAdds < 5; ++numAdds)
            if(c < cUtf8Limits[numAdds])
                break;
        uint32_t value = c - cUtf8Limits[numAdds - 1];

        do
        {
            byte_t c2;
            if(srcPos == maxSrcLen || src[srcPos] == '\0')
                break;
            c2 = src[srcPos++];
            if(c2 < 0x80 || c2 >= 0xC0)
                break;
            value <<= 6;
            value |= (c2 - 0x80);
        }
        while(--numAdds != 0);

        if(value < 0x10000)
        {
            if(dest)
                dest[destPos] = static_cast<wchar_t>(value);
            ++destPos;
        }
        else
        {
            value -= 0x10000;
            if(value >= 0x100000)
                break;
            if(dest)
            {
                dest[destPos + 0] = static_cast<wchar_t>(0xD800 + (value >> 10));
                dest[destPos + 1] = static_cast<wchar_t>(0xDC00 + (value & 0x3FF));
            }
            destPos += 2;
        }
    }

    destLen = destPos;
    return false;
}

bool utf8ToWStr(const char* utf8Str, size_t maxCount, std::wstring& wideStr)
{
    size_t destLen = 0;

// Get the length of the wide string
    if(!utf8ToUtf16(nullptr, destLen, utf8Str, maxCount))
        return false;

    wideStr.resize(destLen);
    if(wideStr.size() != destLen)
        return false;

    return utf8ToUtf16(const_cast<wchar_t*>(wideStr.c_str()), destLen, utf8Str, maxCount);
}

bool utf8ToWStr(StringView utf8Str, std::wstring& wideStr)
{
    return utf8ToWStr(utf8Str.data(), utf8Str.length(), wideStr);
}

//! See the documentation for utf8ToUtf16()
bool utf16ToUtf8(char* dest, size_t& destLen, const wchar_t* src, size_t maxSrcLen)
{
    size_t destPos = 0, srcPos = 0;

    while(true)
    {
        uint32_t value;
        size_t numAdds;

        if(srcPos == maxSrcLen || src[srcPos] == L'\0')
        {
            if(dest && destLen != destPos)
            {
                DFATAL("The provided destLen should equals to what we calculated here");
                return false;
            }
            destLen = destPos;
            return true;
        }

        value = src[srcPos++];

        if(value < 0x80)   // 0-127, US-ASCII (single byte)
        {
            if(dest)
                dest[destPos] = char(value);
            ++destPos;
            continue;
        }

        if(value >= 0xD800 && value < 0xE000)
        {
            if(value >= 0xDC00 || srcPos == maxSrcLen)
                break;
            uint32_t c2 = src[srcPos++];
            if(c2 < 0xDC00 || c2 >= 0xE000)
                break;
            value = ((value - 0xD800) << 10) | (c2 - 0xDC00);
        }

        for(numAdds = 1; numAdds < 5; ++numAdds)
            if(value < (uint32_t(1) << (numAdds * 5 + 6)))
                break;

        if(dest)
            dest[destPos] = char(cUtf8Limits[numAdds - 1] + (value >> (6 * numAdds)));
        ++destPos;

        do
        {
            --numAdds;
            if(dest)
                dest[destPos] = char(0x80 + ((value >> (6 * numAdds)) & 0x3F));
            ++destPos;
        }
        while(numAdds != 0);
    }

    destLen = destPos;
    return false;
}

bool wStrToUtf8(const wchar_t* wideStr, size_t maxCount, String& utf8Str)
{
    size_t destLen = 0;

// Get the length of the utf-8 string
    if(!utf16ToUtf8(nullptr, destLen, wideStr, maxCount))
        return false;

    Vector<char> buf(destLen + 1);
    buf.back() = 0;
//    utf8Str = String(destLen, 0);
//    DCHECK(utf8Str.length() == destLen, "Cannot allocate %d bytes, out of memory?", destLen);

    bool r = utf16ToUtf8(buf.data(), destLen, wideStr, maxCount);
    utf8Str = buf.data();
    return r;
}

bool wStrToUtf8(const std::wstring& wideStr, String& utf8Str)
{
    return wStrToUtf8(wideStr.c_str(), wideStr.size(), utf8Str);
}

String Strings::toUTF8(const std::wstring& text)
{
    String utf8;
    wStrToUtf8(text, utf8);
    return utf8;
}

std::wstring Strings::fromUTF8(const StringView text)
{
    std::wstring wide;
    utf8ToWStr(text, wide);
    return wide;
}

bool Strings::splitFunction(const String& expr, String& func, String& args)
{
    const auto idx = expr.find('(');
    if(idx && idx != String::npos)
    {
        func = expr.substr(0, idx);
        args = expr.substr(idx + 1, expr.length() - 1);
        return isVariableName(func, false) && Strings::parentheses(expr, idx) == expr.length() - 1;
    }
    return false;
}

String Strings::capitalizeFirst(const String& name)
{
    ASSERT(!name.empty());
    String s = name;
    s[0] = static_cast<char>(toupper(s[0]));
    return s;
}

String Strings::sprintf(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    String str = svprintf(format, args);
    va_end(args);
    return str;
}

String Strings::svprintf(const char* format, va_list args)
{
#ifdef __APPLE__
    va_list c;
    va_copy(c, args);
    size_t size = vsnprintf(nullptr, 0, format, c) + 1; // Extra space for '\0
    va_end(c);
#else
#ifndef _MSC_VER
    size_t size = vsnprintf(nullptr, 0, format, args) + 1; // Extra space for '\0
#else
    size_t size = _vscprintf(format, args) + 1;
#endif
#endif
    std::unique_ptr<char[]> buf(new char[size + 1]);
    char* lpstr = buf.get();
    vsnprintf(lpstr, size, format, args);
    lpstr[size] = 0;
    return String(lpstr); // We don't want the '\0' inside
}

String Strings::dumpMemory(const uint8_t* memory, size_t length)
{
    constexpr char padding[] = "         ";
    StringBuffer sb;

    for(size_t i = 0; i < length; i += 16)
    {
        char buf[256];
        std::snprintf(buf, sizeof(buf), "%08Xh: ", static_cast<uint32_t>(i));
        sb << buf;

        for(size_t j = 0; j < 16; j += 4)
        {
            const size_t offset = i + j;
            const int32_t* p1 = reinterpret_cast<const int32_t*>(memory + offset);
            if(offset < length)
            {
                std::snprintf(buf, sizeof(buf), "%08X ", *p1);
                sb << buf;
            }
            else
                sb << padding;
        }

        sb << " |  ";
        for(size_t j = 0; j < 16; ++j)
        {
            size_t offset = i + j;
            if(offset < length)
                sb << (std::isprint(memory[offset]) ? static_cast<char>(memory[offset]) : '.');
            else
                sb << ' ';
        }
        sb << std::endl;
    }

    return sb.str();
}

String Strings::stripReference(const String& id)
{
    DASSERT(id);
    return isVariableCharacter(id.at(0)) ? id : id.substr(1);
}

bool Strings::isNumeric(const String& value)
{
    const char* str = value.c_str();
    while(*str)
        if(const char c = *(str++); c < '+' || c > '9' || c == '/')
            return false;
    return true;
}

bool Strings::isArgument(const String& value)
{
    const char* str = value.c_str();
    if(*str != '$')
        return false;
    return isVariableName(unwrap(str + 1, '{', '}'), false);
}

bool Strings::isVariableCharacter(const char c, const bool allowDash)
{
    if(!((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '_' || c == '.' || (allowDash && c == '-')))
        return false;
    return true;
}

bool Strings::isVariableName(const String& name, const bool allowDash)
{
    const char* str = name.c_str();
    while(*str)
    {
        if(!isVariableCharacter(*(str++), allowDash))
            return false;
    }
    return true;
}

Strings::BUILDER::BUILDER(BeanFactory& /*factory*/, const String& value)
    : _delegate(Strings::load(value))
{
}

sp<String> Strings::BUILDER::build(const Scope& args)
{
    return sp<String>::make(_delegate->build(args));
}

Strings::BUILDER_STR::BUILDER_STR(BeanFactory& /*factory*/, const String& value)
    : _delegate(Strings::load(value))
{
}

String Strings::BUILDER_STR::build(const Scope& args)
{
    return _delegate->build(args);
}

}
