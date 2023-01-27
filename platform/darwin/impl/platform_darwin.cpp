#include "platform/platform.h"

#include <dlfcn.h>
#include <limits.h>
#include <mach-o/dyld.h>
#include <unistd.h>

#ifndef ARK_PLATFORM_IOS
#include <libproc.h>
#endif

#ifdef ARK_USE_OPEN_GL
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

using namespace platform::darwin;

void Platform::log(Log::LogLevel /*logLevel*/, const char* tag, const char* content)
{
    printf("[%s] %s\n", tag, content);
    fflush(nullptr);
}

sp<AssetBundle> Platform::getAssetBundle(const String& path, const String& appPath)
{
    sp<AssetBundle> bundle = AssetBundleDarwin::exists(path) ? sp<AssetBundle>::adopt(new AssetBundleDarwin(path)) : sp<AssetBundle>::null();
    if(isDirectory(path))
    {
        const sp<AssetBundle> pathBundle = sp<AssetBundleDirectory>::make(path);
        bundle = bundle ? sp<AssetBundleWithFallback>::make(bundle, pathBundle).cast<AssetBundle>() : pathBundle;
    }
    if(isDirectory(appPath))
    {
        const sp<AssetBundle> appPathBundle = sp<AssetBundleDirectory>::make(path);
        bundle = bundle ? sp<AssetBundleWithFallback>::make(bundle, appPathBundle).cast<AssetBundle>() : appPathBundle;
    }
    return bundle;
}

sp<Alphabet> Platform::getSystemAlphabet(const Font& font, const String& lang)
{
    if(isFile("/Library/Fonts/Arial Unicode.ttf"))
        return sp<AlphabetTrueType>::make("Arial Unicode.ttf", font.size());
    return sp<AlphabetTrueType>::make("Arial.ttf", font.size());
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
    if(Platform::isFile(path) || Platform::isDirectory(path))
    {
        char buf[PATH_MAX] = {0};
        char* rp = realpath(path.c_str(), buf);
        DCHECK(rp, "realpath failed for \"%s\", errno: %d", path.c_str(), errno);
        return rp;
    }
    return path;
}

std::vector<Ark::RendererVersion> Platform::getRendererVersionPreferences()
{
    return {Ark::RENDERER_VERSION_VULKAN_12, Ark::RENDERER_VERSION_OPENGL_41};
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
#ifdef ARK_USE_OPEN_GL
    glbinding::Binding::initialize(nullptr);
#endif
}

void Platform::vkInitialize()
{
}

String Platform::glShaderVersionDeclaration(uint32_t versionCode)
{
    return Strings::sprintf("#version %d\n", versionCode);
}

uint32_t Platform::glPreprocessShader(const String& shader, const char* srcs[], uint32_t length)
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
