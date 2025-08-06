#include "platform/platform.h"

#include <dlfcn.h>
#include <limits.h>
#include <mach-o/dyld.h>
#include <unistd.h>

#include <CoreFoundation/CFBundle.h>
#include <CoreFoundation/CFStream.h>

#ifndef ARK_PLATFORM_IOS
#include <libproc.h>
#endif

#ifdef ARK_BUILD_PLUGIN_OPENGL
#include <glbinding/gl/gl.h>
#include <glbinding/Binding.h>
#endif

#include "core/impl/asset_bundle/asset_bundle_directory.h"
#include "core/impl/asset_bundle/asset_bundle_with_fallback.h"
#include "core/util/strings.h"

#include "graphics/base/font.h"
#include "graphics/impl/alphabet/alphabet_true_type.h"

#include "impl/asset_bundle_darwin.h"

namespace ark {

namespace {

bool exists(const String& location)
{
    const CFBundleRef bundle = CFBundleGetMainBundle();
    const CFStringRef cfDirectory = CFStringCreateWithCString(nullptr, location.c_str(), kCFStringEncodingUTF8);
    const CFURLRef url = CFBundleCopyResourceURL(bundle, nullptr, nullptr, cfDirectory);
    CFRelease(cfDirectory);
    const bool r = static_cast<bool>(url);
    if(url)
        CFRelease(url);
    return r;
}

}

using namespace platform::darwin;

sp<AssetBundle> Platform::getAssetBundle(const String& path, const String& appPath)
{
    sp<AssetBundle> bundle = exists(path) ? sp<AssetBundle>::make<AssetBundleDarwin>(path) : nullptr;
    if(isDirectory(path))
    {
        const sp<AssetBundle> pathBundle = sp<AssetBundle>::make<AssetBundleDirectory>(path);
        bundle = bundle ? sp<AssetBundle>::make<AssetBundleWithFallback>(std::move(bundle), std::move(pathBundle)) : std::move(pathBundle);
    }
    if(isDirectory(appPath))
    {
        const sp<AssetBundle> appPathBundle = sp<AssetBundle>::make<AssetBundleDirectory>(path);
        bundle = bundle ? sp<AssetBundle>::make<AssetBundleWithFallback>(std::move(bundle), std::move(appPathBundle)) : std::move(appPathBundle);
    }
    return bundle;
}

sp<Alphabet> Platform::getSystemAlphabet(const Font& font, const String& lang)
{
    if(isFile("/Library/Fonts/Arial Unicode.ttf"))
        return sp<Alphabet>::make<AlphabetTrueType>("Arial Unicode.ttf");
    return sp<Alphabet>::make<AlphabetTrueType>("Arial.ttf");
}

String Platform::getDefaultFontDirectory()
{
    return "/Library/Fonts";
}

String Platform::getExecutablePath()
{
    char pathbuf[PATH_MAX] = {0};
#ifndef ARK_PLATFORM_IOS
    const pid_t pid = getpid();
    int r = proc_pidpath (pid, pathbuf, sizeof(pathbuf));
    DCHECK(r > 0, "proc_pidpath() failed. PID %d: %s", pid, strerror(errno));
#endif
    return pathbuf;
}

String Platform::getRealPath(const String& path)
{
    if(isFile(path) || isDirectory(path))
    {
        char buf[PATH_MAX] = {0};
        char* rp = realpath(path.c_str(), buf);
        DCHECK(rp, "realpath failed for \"%s\", errno: %d", path.c_str(), errno);
        return rp;
    }
    return path;
}

Vector<enums::RendererVersion> Platform::getRendererVersionPreferences()
{
    return {enums::RENDERER_VERSION_VULKAN_12, enums::RENDERER_VERSION_OPENGL_41};
}

String Platform::getUserStoragePath(const String& path)
{
    return Strings::sprintf("%s/Library/Application Support/%s", getenv("HOME"), path.c_str());
}

String Platform::getExternalStoragePath(const String& path)
{
    return path;
}

void Platform::sysCall(int32_t /*id*/, const String& /*value*/)
{
}

void Platform::glInitialize()
{
}

void Platform::vkInitialize()
{
}

String Platform::glShaderVersionDeclaration(const uint32_t versionCode)
{
    return Strings::sprintf("#version %d\n", versionCode);
}

uint32_t Platform::glPreprocessShader(const String& shader, const char* srcs[], const uint32_t length)
{
    DCHECK(length > 0, "length = %d", length);
    srcs[0] = shader.c_str();
    return 1;
}

void* Platform::dlOpen(const char* name)
{
    if(!name)
        return dlopen(nullptr, RTLD_LAZY);

    const String sFilePath = getExecutablePath();
    const String soName = Strings::sprintf("lib%s.dylib", name);
    const auto [sDirname, sFilename] = sFilePath.rcut(dirSeparator());
    void* library = dlopen(soName.c_str(), RTLD_LAZY);
    if(!library && sDirname)
        library = dlopen((sDirname.value() + "/" + soName).c_str(), RTLD_LAZY);
    return library;
}

void* Platform::dlSymbol(void* library, const String& symbolName)
{
    return dlsym(library, symbolName.c_str());
}

void Platform::dlClose(void* library)
{
    dlclose(library);
}

void Platform::detachCurrentThread()
{
}

char PlatformIndependent::_DIR_SEPARATOR = '/';
char PlatformIndependent::_PATH_SEPARATOR = ':';

}
