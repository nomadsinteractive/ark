#include "core/util/conversions.h"

#include <string>
#include <stdlib.h>

#include "core/ark.h"
#include "core/base/api.h"
#include "core/base/clock.h"
#include "core/base/string_buffer.h"
#include "core/inf/variable.h"
#include "core/inf/array.h"
#include "core/util/strings.h"
#include "core/types/shared_ptr.h"

#include "app/view/layout_param.h"

namespace ark {

template<> ARK_API uint32_t Conversions::to<String, uint32_t>(const String& str)
{
    return atoi(str.c_str());
}

template<> ARK_API int32_t Conversions::to<String, int32_t>(const String& str)
{
    return atoi(str.c_str());
}

template<> ARK_API uint16_t Conversions::to<String, uint16_t>(const String& str)
{
    return static_cast<uint16_t>(atoi(str.c_str()));
}

template<> ARK_API int16_t Conversions::to<String, int16_t>(const String& str)
{
    return static_cast<int16_t>(atoi(str.c_str()));
}

template<> ARK_API float Conversions::to<String, float>(const String& str)
{
    if(str == "match_parent")
        return LayoutParam::MATCH_PARENT;
    if(str == "wrap_content")
        return LayoutParam::WRAP_CONTENT;
    if(str.endsWith("s"))
        return Conversions::to<String, Clock::Interval>(str).sec();
    return static_cast<float>(atof(str.c_str()));
}

template<> ARK_API Ark::RendererVersion Conversions::to<String, Ark::RendererVersion>(const String& str)
{
    const String glversion = str.toLower();
    if(glversion.startsWith("opengl_"))
        return static_cast<Ark::RendererVersion>(atoi(glversion.c_str() + 7));
    if(glversion.startsWith("vulkan_"))
        return static_cast<Ark::RendererVersion>(atoi(glversion.c_str() + 7) + 100);
    WARN(false, "Unknow GLVersion: \"%s, supported GLVersions are [\"opengl_21\", \"opengl_46\", \"vulkan_11\", ...]", str.c_str());
    return Ark::AUTO;
}

template<> ARK_API String Conversions::to<uint32_t, String>(const uint32_t& val)
{
    return Strings::sprintf("%d", val);
}

template<> ARK_API String Conversions::to<int32_t, String>(const int32_t& val)
{
    return Strings::sprintf("%d", val);
}

template<> ARK_API String Conversions::to<uint16_t, String>(const uint16_t& val)
{
    return Strings::sprintf("%d", val);
}

template<> ARK_API String Conversions::to<int16_t, String>(const int16_t& val)
{
    return Strings::sprintf("%d", val);
}

template<> ARK_API String Conversions::to<float, String>(const float& val)
{
    return Strings::sprintf("%.2f", val);
}

template<> ARK_API bool Conversions::to<String, bool>(const String& str)
{
    if(str == "true")
        return true;
    if(str == "false")
        return false;
    return !str.empty();
}

template<> ARK_API String Conversions::to<bool, String>(const bool& val)
{
    return val ? "true" : "false";
}

template<> ARK_API std::wstring Conversions::to<String, std::wstring>(const String& str)
{
    return Strings::fromUTF8(str);
}

template<> ARK_API String Conversions::to<std::wstring, String>(const std::wstring& str)
{
    return Strings::toUTF8(str);
}

template<> ARK_API sp<Numeric> Conversions::to<float, sp<Numeric>>(const float& val)
{
    return sp<Numeric::Impl>::make(val);
}

template<> ARK_API float Conversions::to<sp<Numeric>, float>(const sp<Numeric>& val)
{
    return val->val();
}

template<> ARK_API String Conversions::to<floatarray, String>(const floatarray& val)
{
    StringBuffer sb;
    sb << '[' << Strings::join<float>(val->buf(), 0, val->length()) << ']';
    return sb.str();
}

template<> ARK_API String Conversions::to<indexarray, String>(const indexarray& val)
{
    StringBuffer sb;
    sb << '[' << Strings::join<uint16_t>(val->buf(), 0, val->length()) << ']';
    return sb.str();
}

}
