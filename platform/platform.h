#ifndef ARK_PLATFORMS_PLATFORM_H_
#define ARK_PLATFORMS_PLATFORM_H_

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/util/log.h"

#include "graphics/forwarding.h"

#include "renderer/forwarding.h"

#include "platform/any/platform_independent.h"

namespace ark {

class ARK_API Platform : public PlatformIndependent {
public:
    static void log(Log::LogLevel logLevel, const char* tag, const char* content);

//  [[script::bindings::auto]]
    static String getUserStoragePath(const String& path);

    static sp<AssetBundle> getAsset(const String& path, const String& appPath);
    static sp<Alphabet> getSystemAlphabet(const Font& font, const String& lang);
    static String getDefaultFontDirectory();
    static String getExecutablePath();
    static String getRealPath(const String& path);

    static void glInitialize();
    static void vkInitialize();

    static String glShaderVersionDeclaration(uint32_t versionCode);
    static uint32_t glPreprocessShader(const String& shader, const char* srcs[], uint32_t length);

    static void* dlOpen(const String& name);
    static void* dlSymbol(void* library, const String& symbolName);
    static void dlClose(void* library);

    static void detachCurrentThread();

};

}

#endif
