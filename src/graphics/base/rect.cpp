#include "graphics/base/rect.h"

#include "core/types/shared_ptr.h"
#include "core/util/string_convert.h"

#include "graphics/base/v4.h"

namespace ark {

template<> ARK_API Rect StringConvert::to<String, Rect>(const String& str)
{
    const std::vector<String> m = Strings::unwrap(str.strip(), '(', ')').split(',');
    DCHECK(m.size() == 4, "Cannot parse Rect from \"%s\"", str.c_str());
    float left = Strings::parse<float>(m[0]);
    float top = Strings::parse<float>(m[1]);
    float right = Strings::parse<float>(m[2]);
    float bottom = Strings::parse<float>(m[3]);
    return Rect(left, top, right, bottom);
}

template<> ARK_API String StringConvert::to<Rect, String>(const Rect& rect)
{
    return Strings::sprintf("(%.2f, %.2f, %.2f, %.2f)", rect.left(), rect.top(), rect.right(), rect.bottom());
}

}
