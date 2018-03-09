#include "platform/platform.h"

#include <windows.h>
#include <shlwapi.h>
#include <shlobj.h>

#include <glbinding/gl/gl.h>
#include <glbinding/Binding.h>

#include "core/inf/variable.h"
#include "core/impl/asset/directory_asset.h"
#include "core/impl/dictionary/dictionary_with_fallback.h"
#include "core/util/strings.h"

#include "graphics/base/font.h"
#include "graphics/impl/alphabet/alphabet_true_type.h"

#pragma comment(lib, "shlwapi.lib")

namespace ark {

void Platform::log(Log::LogLevel /*logLevel*/, const char* tag, const char* content)
{
    printf("[%s] %s\n", tag, content);
    fflush(nullptr);
}

sp<Asset> Platform::getAsset(const String& path, const String& appPath)
{
    if(isDirectory(path))
        return sp<DictionaryWithFallback<sp<Readable>>>::make(sp<DirectoryAsset>::make(path), sp<DirectoryAsset>::make(appPath));
    if(isDirectory(appPath))
        return sp<DirectoryAsset>::make(appPath);
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

//static int CALLBACK _enumerate_font_callback(CONST LOGFONT* lpelfe, CONST TEXTMETRIC* lpntme, DWORD FontType, LPARAM lParam)
//{
//    puts(lpelfe->lfFaceName);
//    return 1;
//}

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
        return sp<AlphabetTrueType>::make("ArialUni.ttf", font.size());
    if(isFile(fontsDirectory + "\\simfang.ttf"))
        return sp<AlphabetTrueType>::make("simfang.ttf", font.size());
    if(isFile(fontsDirectory + "\\Arial.ttf"))
        return sp<AlphabetTrueType>::make("Arial.ttf", font.size());
    return sp<AlphabetTrueType>::make("l_10646.ttf", font.size());
}

void Platform::glInitialize()
{
    glbinding::Binding::initialize(false);
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
    const String dllName = name + String(".dll");
    return LoadLibrary(dllName.c_str());
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
