#ifndef ARK_PLATFORM_ANDROID_UTIL_FONT_CONFIG_H_
#define ARK_PLATFORM_ANDROID_UTIL_FONT_CONFIG_H_

#include "core/forwarding.h"

#include "graphics/forwarding.h"

namespace ark {
namespace platform {
namespace android {

void _loadSystemFontWithFallback(const Font& font, const String& lang, String& alphabet, String& fallback);

}
}
}
#endif
