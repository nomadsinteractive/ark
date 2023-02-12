#include "core/util/string_convert.h"

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

template<> ARK_API String StringConvert::to<String, String>(const String& str)
{
    return str;
}

template<> ARK_API uint32_t StringConvert::to<String, uint32_t>(const String& str)
{
    return atoi(str.c_str());
}

template<> ARK_API int32_t StringConvert::to<String, int32_t>(const String& str)
{
    return atoi(str.c_str());
}

template<> ARK_API uint16_t StringConvert::to<String, uint16_t>(const String& str)
{
    return static_cast<uint16_t>(atoi(str.c_str()));
}

template<> ARK_API int16_t StringConvert::to<String, int16_t>(const String& str)
{
    return static_cast<int16_t>(atoi(str.c_str()));
}

template<> ARK_API float StringConvert::to<String, float>(const String& str)
{
    if(str == "match_parent")
        return static_cast<float>(LayoutParam::SIZE_CONSTRAINT_MATCH_PARENT);
    if(str == "wrap_content")
        return static_cast<float>(LayoutParam::SIZE_CONSTRAINT_WRAP_CONTENT);
    if(str.endsWith("s"))
        return StringConvert::to<String, Clock::Interval>(str).sec();
    return static_cast<float>(atof(str.c_str()));
}

template<> Ark::RendererVersion StringConvert::to<String, Ark::RendererVersion>(const String& str)
{
    const String version = str.toLower();
    if(version.startsWith("opengl_"))
        return static_cast<Ark::RendererVersion>(atoi(version.c_str() + 7));
    if(version.startsWith("vulkan_"))
        return static_cast<Ark::RendererVersion>(atoi(version.c_str() + 7) + 100);
    CHECK_WARN(str == "auto", "Unknow RendererVersion: \"%s, supported values are [\"opengl_21\", \"opengl_46\", \"vulkan_11\", ...]", str.c_str());
    return Ark::RENDERER_VERSION_AUTO;
}

template<> Ark::RendererCoordinateSystem StringConvert::to<String, Ark::RendererCoordinateSystem>(const String& str)
{
    const String cs = str.toLower();
    if(cs == "lhs")
        return Ark::COORDINATE_SYSTEM_LHS;
    DCHECK(str == "rhs", "Unknow RendererCoordinateSystem: \"%s, supported values are [\"lhs\", \"rhs\"]", str.c_str());
    return Ark::COORDINATE_SYSTEM_RHS;
}

template<> ARK_API String StringConvert::to<uint32_t, String>(const uint32_t& val)
{
    return Strings::sprintf("%d", val);
}

template<> ARK_API String StringConvert::to<int32_t, String>(const int32_t& val)
{
    return Strings::sprintf("%d", val);
}

template<> ARK_API String StringConvert::to<uint16_t, String>(const uint16_t& val)
{
    return Strings::sprintf("%d", val);
}

template<> ARK_API String StringConvert::to<int16_t, String>(const int16_t& val)
{
    return Strings::sprintf("%d", val);
}

template<> ARK_API String StringConvert::to<float, String>(const float& val)
{
    return Strings::sprintf("%.2f", val);
}

template<> ARK_API bool StringConvert::to<String, bool>(const String& str)
{
    if(str == "true")
        return true;
    if(str == "false")
        return false;
    return !str.empty();
}

template<> ARK_API String StringConvert::to<bool, String>(const bool& val)
{
    return val ? "true" : "false";
}

template<> ARK_API String StringConvert::to<V2, String>(const V2& val)
{
    return Strings::sprintf("(%.2f, %.2f)", val.x(), val.y());
}

template<> ARK_API String StringConvert::to<V3, String>(const V3& val)
{
    return Strings::sprintf("(%.2f, %.2f, %.2f)", val.x(), val.y(), val.z());
}

template<> ARK_API String StringConvert::to<V4, String>(const V4& val)
{
    return Strings::sprintf("(%.2f, %.2f, %.2f, %.2f)", val.x(), val.y(), val.z(), val.w());
}

template<> ARK_API String StringConvert::to<sp<String>, String>(const sp<String>& val)
{
    return *val;
}

template<> ARK_API std::wstring StringConvert::to<String, std::wstring>(const String& str)
{
    return Strings::fromUTF8(str);
}

template<> ARK_API String StringConvert::to<std::wstring, String>(const std::wstring& str)
{
    return Strings::toUTF8(str);
}

template<> String StringConvert::to<floatarray, String>(const floatarray& val)
{
    StringBuffer sb;
    sb << '[' << Strings::join<float>(val->buf(), 0, val->length()) << ']';
    return sb.str();
}

template<> String StringConvert::to<indexarray, String>(const indexarray& val)
{
    StringBuffer sb;
    sb << '[' << Strings::join<uint16_t>(val->buf(), 0, val->length()) << ']';
    return sb.str();
}

template<> String StringConvert::to<bytearray, String>(const bytearray& val)
{
    return Strings::dumpMemory(val->buf(), val->size());
}

}
