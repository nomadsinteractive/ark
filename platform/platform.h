#pragma once

#include "core/ark.h"
#include "core/forwarding.h"
#include "core/base/api.h"

#include "graphics/forwarding.h"

#include "platform/any/platform_independent.h"

namespace ark {

class ARK_API Platform : public PlatformIndependent {
public:
//  [[script::bindings::auto]]
    static String getUserStoragePath(const String& path);
//  [[script::bindings::auto]]
    static String getExternalStoragePath(const String& path);
//  [[script::bindings::auto]]
    static void sysCall(int32_t id, const String& value);

    static sp<AssetBundle> getAssetBundle(const String& path);
    static String getDefaultFontDirectory();
    static String getExecutablePath();
    static String getRealPath(const String& path);
    static Vector<enums::RendererVersion> getRendererVersionPreferences();

    static String glShaderVersionDeclaration(uint32_t versionCode);
    static uint32_t glPreprocessShader(const String& shader, const char* srcs[], uint32_t length);

    static void* dlOpen(const char* name);
    static void* dlSymbol(void* library, const String& symbolName);
    static void dlClose(void* library);

    static void detachCurrentThread();

};

}
