#include "platform/platform.h"

#include <dlfcn.h>
#include <limits.h>
#include <mach-o/dyld.h>
#include <unistd.h>
#include <libproc.h>

#include <glbinding/gl/gl.h>
#include <glbinding/Binding.h>

#include "core/inf/variable.h"
#include "core/impl/asset_bundle/asset_bundle_directory.h"
#include "core/impl/asset_bundle/asset_bundle_with_fallback.h"
#include "core/impl/dictionary/xml_directory.h"
#include "core/util/strings.h"

#include "graphics/base/font.h"
#include "graphics/impl/alphabet/alphabet_true_type.h"

namespace ark {

void Platform::log(Log::LogLevel /*logLevel*/, const char* tag, const char* content)
{
    printf("[%s] %s\n", tag, content);
    fflush(nullptr);
}

sp<AssetBundle> Platform::getAsset(const String& path, const String& appPath)
{
    if(isDirectory(path))
        return sp<AssetBundleWithFallback>::make(sp<AssetBundleDirectory>::make(appPath), sp<AssetBundleDirectory>::make(path));
    if(isDirectory(appPath))
        return sp<AssetBundleDirectory>::make(appPath);
    return nullptr;
}

sp<Alphabet> Platform::getSystemAlphabet(const Font& font, const String& lang)
{
    if(lang && isFile("/Library/Fonts/Arial Unicode.ttf"))
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
    const pid_t pid = getpid();
    int r = proc_pidpath (pid, pathbuf, sizeof(pathbuf));
    DCHECK(r > 0, "proc_pidpath() failed. PID %d: %s", pid, strerror(errno));
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

String Platform::getUserStoragePath(const String& path)
{
    return Strings::sprintf("%s/Library/Application Support/%s", getenv("HOME"), path.c_str());
}

void Platform::glInitialize()
{
    glbinding::Binding::initialize(false);
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

void* Platform::dlOpen(const String& name)
{
    String sDirname, sFilename;
    const String sFilePath = getExecutablePath();
    const String soName = Strings::sprintf("lib%s.dylib", name.c_str());
    Strings::rcut(sFilePath, sDirname, sFilename, dirSeparator());
    void* library = dlopen(soName.c_str(), RTLD_LAZY);
    if(!library)
        library = dlopen((sDirname + "/" + soName).c_str(), RTLD_LAZY);
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
