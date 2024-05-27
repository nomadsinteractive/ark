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

template<> ARK_API String StringConvert::eval<String>(const String& repr)
{
    return repr;
}

template<> ARK_API String StringConvert::repr<String>(const String& obj)
{
    return obj;
}

template<> ARK_API uint32_t StringConvert::eval<uint32_t>(const String& repr)
{
    return atoi(repr.c_str());
}

template<> ARK_API int32_t StringConvert::eval<int32_t>(const String& repr)
{
    return atoi(repr.c_str());
}

template<> ARK_API uint16_t StringConvert::eval<uint16_t>(const String& repr)
{
    return static_cast<uint16_t>(atoi(repr.c_str()));
}

template<> ARK_API int16_t StringConvert::eval<int16_t>(const String& str)
{
    return static_cast<int16_t>(atoi(str.c_str()));
}

template<> ARK_API float StringConvert::eval<float>(const String& repr)
{
    if(repr.endsWith("s"))
        return StringConvert::eval<Clock::Interval>(repr).sec();
    return static_cast<float>(atof(repr.c_str()));
}

template<> Ark::RendererVersion StringConvert::eval<Ark::RendererVersion>(const String& repr)
{
    const String version = repr.toLower();
    if(version.startsWith("opengl_"))
        return static_cast<Ark::RendererVersion>(atoi(version.c_str() + 7));
    if(version.startsWith("vulkan_"))
        return static_cast<Ark::RendererVersion>(atoi(version.c_str() + 7) + 100);
    CHECK(repr == "auto", "Unknow RendererVersion: \"%s, supported values are [\"opengl_21\", \"opengl_46\", \"vulkan_11\", ...]", repr.c_str());
    return Ark::RENDERER_VERSION_AUTO;
}

template<> Ark::RendererCoordinateSystem StringConvert::eval<Ark::RendererCoordinateSystem>(const String& str)
{
    const String cs = str.toLower();
    if(cs == "lhs")
        return Ark::COORDINATE_SYSTEM_LHS;
    DCHECK(str == "rhs", "Unknow RendererCoordinateSystem: \"%s, supported values are [\"lhs\", \"rhs\"]", str.c_str());
    return Ark::COORDINATE_SYSTEM_RHS;
}

template<> ARK_API String StringConvert::repr<uint32_t>(const uint32_t& val)
{
    return Strings::sprintf("%d", val);
}

template<> ARK_API String StringConvert::repr<int32_t>(const int32_t& val)
{
    return Strings::sprintf("%d", val);
}

template<> ARK_API String StringConvert::repr<uint16_t>(const uint16_t& val)
{
    return Strings::sprintf("%d", val);
}

template<> ARK_API String StringConvert::repr<int16_t>(const int16_t& val)
{
    return Strings::sprintf("%d", val);
}

template<> ARK_API String StringConvert::repr<float>(const float& val)
{
    return Strings::sprintf("%.2f", val);
}

template<> ARK_API bool StringConvert::eval<bool>(const String& str)
{
    if(str == "true")
        return true;
    if(str == "false")
        return false;
    return !str.empty();
}

template<> ARK_API String StringConvert::repr<bool>(const bool& val)
{
    return val ? "true" : "false";
}

template<> ARK_API String StringConvert::repr<V2>(const V2& val)
{
    return Strings::sprintf("(%.2f, %.2f)", val.x(), val.y());
}

template<> ARK_API String StringConvert::repr<V3>(const V3& val)
{
    return Strings::sprintf("(%.2f, %.2f, %.2f)", val.x(), val.y(), val.z());
}

template<> ARK_API String StringConvert::repr<V4>(const V4& val)
{
    return Strings::sprintf("(%.2f, %.2f, %.2f, %.2f)", val.x(), val.y(), val.z(), val.w());
}

template<> ARK_API String StringConvert::repr<sp<String>>(const sp<String>& val)
{
    return *val;
}

template<> ARK_API std::wstring StringConvert::eval<std::wstring>(const String& str)
{
    return Strings::fromUTF8(str);
}

template<> ARK_API String StringConvert::repr<std::wstring>(const std::wstring& str)
{
    return Strings::toUTF8(str);
}

template<> String StringConvert::repr<floatarray>(const floatarray& val)
{
    StringBuffer sb;
    sb << '[' << Strings::join<float>(val->buf(), 0, val->length()) << ']';
    return sb.str();
}

template<> String StringConvert::repr<indexarray>(const indexarray& val)
{
    StringBuffer sb;
    sb << '[' << Strings::join<uint16_t>(val->buf(), 0, val->length()) << ']';
    return sb.str();
}

template<> String StringConvert::repr<bytearray>(const bytearray& val)
{
    return Strings::dumpMemory(val->buf(), val->size());
}

}
