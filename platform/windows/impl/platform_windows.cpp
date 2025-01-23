#include "platform/platform.h"

#include <windows.h>
#include <shlwapi.h>
#include <shlobj.h>

#include <glbinding/gl/gl.h>
#include <glbinding/Binding.h>

#include "core/inf/variable.h"
#include "core/impl/asset_bundle/asset_bundle_with_fallback.h"
#include "core/impl/asset_bundle/asset_bundle_directory.h"

#include "graphics/base/font.h"
#include "graphics/impl/alphabet/alphabet_true_type.h"

#pragma comment(lib, "shlwapi.lib")

namespace ark {

void Platform::log(Log::LogLevel /*logLevel*/, const char* tag, const char* content)
{
    printf("[%s] %s\n", tag, content);
    fflush(nullptr);
}

sp<AssetBundle> Platform::getAssetBundle(const String& path, const String& appPath)
{
    if(isDirectory(path))
        return sp<AssetBundleWithFallback>::make(sp<AssetBundleDirectory>::make(appPath), sp<AssetBundleDirectory>::make(path));
    if(isDirectory(appPath))
        return sp<AssetBundleDirectory>::make(appPath);
    return nullptr;
}

String Platform::getUserStoragePath(const String& path)
{
    wchar_t* szPath = nullptr;
    if (!SUCCEEDED(SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, NULL, &szPath)))
        FATAL("Get application local storage path failed");
    const String appPath = Strings::toUTF8(szPath);
    CoTaskMemFree(szPath);
    return Strings::sprintf("%s\\%s", appPath.c_str(), path.c_str());
}

String Platform::getExternalStoragePath(const String& path)
{
    const String s = path.rstrip('/').rstrip('\\').lstrip('/').lstrip('\\');
    return s.empty() ? "." : s;
}

void Platform::sysCall(int32_t id, const String& value)
{
}

String Platform::getDefaultFontDirectory()
{
    TCHAR nWindowsDirectory[MAX_PATH];
    GetWindowsDirectory(nWindowsDirectory, MAX_PATH);
    return Strings::sprintf("%s\\%s", nWindowsDirectory, "fonts");
}

String Platform::getExecutablePath()
{
    TCHAR pathbuf[MAX_PATH];
    int32_t bytes = GetModuleFileName(NULL, pathbuf, sizeof(pathbuf));
    DCHECK(bytes > 0, "GetModuleFileName() failed");
    return pathbuf;
}

String Platform::getRealPath(const String& path)
{
    char realpath[MAX_PATH];
    GetFullPathName(path.c_str(), sizeof(realpath), realpath, nullptr);
    return realpath;
}

std::vector<Ark::RendererVersion> Platform::getRendererVersionPreferences()
{
    return {Ark::RENDERER_VERSION_OPENGL_46, Ark::RENDERER_VERSION_VULKAN_12};
}

sp<Alphabet> Platform::getSystemAlphabet(const Font& font, const String& lang)
{
/*
 * Lucida Sans Unicode
 * l_10646.ttf
*/
//    LOGFONT font = {0};
//    HWND hwnd = GetDesktopWindow();
//    HDC hdc = GetWindowDC(hwnd);
//    font.lfCharSet = DEFAULT_CHARSET;
//    EnumFontFamiliesEx(hdc, &font, _enumerate_font_callback, 0, 0);
    const String fontsDirectory = getDefaultFontDirectory();
    if(isFile(fontsDirectory + "\\ArialUni.ttf"))
        return sp<AlphabetTrueType>::make("ArialUni.ttf");
    if(isFile(fontsDirectory + "\\simfang.ttf"))
        return sp<AlphabetTrueType>::make("simfang.ttf");
    if(isFile(fontsDirectory + "\\Arial.ttf"))
        return sp<AlphabetTrueType>::make("Arial.ttf");

    return sp<AlphabetTrueType>::make("l_10646.ttf");
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
    if(name)
    {
        if(String(name).endsWith(".dll"))
            LoadLibrary(name);
        return LoadLibrary(Strings::sprintf("%s.dll", name).c_str());
    }
    HMODULE hModule = nullptr;
    GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCTSTR)Platform::dlOpen, &hModule);
    return hModule;
}

void* Platform::dlSymbol(void* library, const String& symbolName)
{
    return GetProcAddress(reinterpret_cast<HMODULE>(library), symbolName.c_str());
}

void Platform::dlClose(void* library)
{
    FreeLibrary(reinterpret_cast<HMODULE>(library));
}

void Platform::detachCurrentThread()
{
}

char PlatformIndependent::_DIR_SEPARATOR = '\\';
char PlatformIndependent::_PATH_SEPARATOR = ';';

}
