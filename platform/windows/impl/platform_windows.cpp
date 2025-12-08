#include "platform/platform.h"

#include <windows.h>
#include <shlwapi.h>
#include <shlobj.h>

#include "core/impl/asset_bundle/asset_bundle_directory.h"
#include "core/util/strings.h"

#include "graphics/impl/alphabet/alphabet_true_type.h"

#pragma comment(lib, "shlwapi.lib")

namespace ark {

sp<AssetBundle> Platform::getAssetBundle(const String& path)
{
    if(isDirectory(path))
        return sp<AssetBundle>::make<AssetBundleDirectory>(path);
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

String Platform::getExecutablePath()
{
    TCHAR pathbuf[MAX_PATH];
    const int32_t bytes = GetModuleFileName(nullptr, pathbuf, sizeof(pathbuf));
    DCHECK(bytes > 0, "GetModuleFileName() failed");
    return pathbuf;
}

String Platform::getRealPath(const String& path)
{
    char realpath[MAX_PATH];
    GetFullPathName(path.c_str(), sizeof(realpath), realpath, nullptr);
    return realpath;
}

Vector<enums::RenderingBackendBit> Platform::getPreferedRenderBackends()
{
#ifdef ARK_FLAG_PUBLISHING_BUILD
    return {enums::RENDERING_BACKEND_BIT_DIRECT_X, enums::RENDERING_BACKEND_BIT_VULKAN, enums::RENDERING_BACKEND_BIT_OPENGL};
#else
    // We need a bit of variety while debugging
    return {enums::RENDERING_BACKEND_BIT_OPENGL, enums::RENDERING_BACKEND_BIT_VULKAN, enums::RENDERING_BACKEND_BIT_DIRECT_X};
#endif
}

String Platform::glShaderVersionDeclaration(const uint32_t versionCode)
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
    return GetProcAddress(static_cast<HMODULE>(library), symbolName.c_str());
}

void Platform::dlClose(void* library)
{
    FreeLibrary(static_cast<HMODULE>(library));
}

void Platform::detachCurrentThread()
{
}

char PlatformIndependent::_DIR_SEPARATOR = '\\';
char PlatformIndependent::_PATH_SEPARATOR = ';';

}
