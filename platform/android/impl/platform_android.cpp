#include "impl/platform_android.h"
#include "platform/platform.h"

#include <android/asset_manager_jni.h>
#include <android/log.h>

#include <dlfcn.h>

#include "core/impl/asset_bundle/asset_bundle_directory.h"
#include "core/impl/dictionary/xml_directory.h"
#include "core/inf/variable.h"
#include "core/inf/readable.h"
#include "core/types/global.h"
#include "core/util/strings.h"

#include "graphics/base/font.h"
#include "graphics/impl/alphabet/alphabet_true_type.h"
#include "graphics/impl/alphabet/alphabet_with_fallback.h"

#include "platform/android/impl/asset_bundle/asset_bundle_android.h"
#include "platform/android/util/jni_util.h"
#include "platform/android/util/font_config.h"

#ifdef ARK_BUILD_PLUGIN_VULKAN
#include "platform/vulkan/vulkan.h"
#endif

using namespace ark::platform::android;

namespace ark {
namespace platform {
namespace android {

ARK_API jobject gAssetManager = nullptr;
ARK_API jobject gApplicationContext = nullptr;

bool directoryExists(AAssetManager* assetManager, const String& path) {
	if(path == ".")
		return true;
    AAssetDir* assetDir = AAssetManager_openDir(assetManager, path.c_str() );
    bool r = AAssetDir_getNextFileName(assetDir) != nullptr;
    AAssetDir_close( assetDir );
    return r;
}

PlatformAndroid::PlatformAndroid(android_app* state)
	: _state(state), _asset_manager(_state->activity->assetManager)
{
}

android_app* PlatformAndroid::state()
{
	return _state;
}

sp<AssetBundle> PlatformAndroid::getAssetBundle(const String& path, const String& appPath)
{
	bool dirExists = directoryExists(_asset_manager, path);
	return dirExists ? sp<AssetBundle>::adopt(new AssetBundleAndroid(_asset_manager, path)) : nullptr;
}

String PlatformAndroid::getUserStoragePath(const String& filename)
{
	if(_state->activity->internalDataPath)
		return Platform::pathJoin(_state->activity->internalDataPath, filename);
	if(_state->activity->externalDataPath)
		return Platform::pathJoin(_state->activity->externalDataPath, filename);
	return ".";
}

}
}
}

namespace ark {

static String java_File_getAbsolutePath(JNIEnv* env, jobject jFile)
{
    jclass jFileClass = env->GetObjectClass(jFile);
    jmethodID getAbsoluteMethodID = env->GetMethodID(jFileClass, "getAbsolutePath", "()Ljava/lang/String;");
    jstring jAbsolutePath = (jstring) env->CallObjectMethod(jFile, getAbsoluteMethodID);
    jboolean isCopy = false;
    const char* buf = env->GetStringUTFChars(jAbsolutePath, &isCopy);
    const String path = buf;
    env->ReleaseStringUTFChars(jAbsolutePath, buf);
    env->DeleteLocalRef(jFileClass);
    env->DeleteLocalRef(jAbsolutePath);
    return path;
}

void Platform::log(Log::LogLevel logLevel, const char* tag, const char* content)
{
	const String message = Strings::sprintf("%s: %s", tag, content);
    __android_log_write(logLevel == Log::LOG_LEVEL_WARNING ? ANDROID_LOG_WARN : (logLevel == Log::LOG_LEVEL_DEBUG ? ANDROID_LOG_DEBUG : ANDROID_LOG_ERROR), LOG_TAG, message.c_str());
}

sp<AssetBundle> Platform::getAssetBundle(const String& path, const String& appPath)
{
	if(gAssetManager)
	{
		JNIEnv* env = JNIUtil::attachCurrentThread();
		AAssetManager* am = AAssetManager_fromJava(env, gAssetManager);
		bool dirExists = directoryExists(am, path);
		return dirExists ? sp<AssetBundle>::adopt(new AssetBundleAndroid(am, path)) : nullptr;
	}
	Global<PlatformAndroid> platformAndroid;
    return platformAndroid->getAssetBundle(path, appPath);
}

void Platform::glInitialize()
{
}

void Platform::vkInitialize()
{
#ifdef ARK_BUILD_PLUGIN_VULKAN
	platform::android::vkInitialize();
#endif
}

String Platform::getUserStoragePath(const String& filename)
{
	if(gApplicationContext)
	{
		String ret = ".";
		JNIEnv* env = JNIUtil::attachCurrentThread();
		int pos = filename.find('/');
		const String path = pos != -1 ? filename.substr(0, pos) : filename;
		const String name = pos != -1 ? filename.substr(pos + 1) : "";
		jclass applicationContextClass = env->GetObjectClass(gApplicationContext);
		jmethodID methodID = env->GetMethodID(applicationContextClass, "getDir", "(Ljava/lang/String;I)Ljava/io/File;");
		jstring jPath = JNIUtil::newStringUTF(env, path.c_str());
		jobject jFile = env->CallObjectMethod(gApplicationContext, methodID, jPath, 0);
		if(jFile)
        {
            ret = pathJoin(java_File_getAbsolutePath(env, jFile), path);
            env->DeleteLocalRef(jFile);
        }
        env->DeleteLocalRef(jPath);
		env->DeleteLocalRef(applicationContextClass);
		return ret;
	}
	Global<PlatformAndroid> platformAndroid;
    return platformAndroid->getUserStoragePath(filename);
}

String Platform::getExternalStoragePath(const String& path)
{
    String ret = ".";
    if(gApplicationContext)
    {
        JNIEnv* env = JNIUtil::attachCurrentThread();
        jclass applicationContextClass = env->GetObjectClass(gApplicationContext);
        jmethodID methodID = env->GetMethodID(applicationContextClass, "getExternalFilesDir", "(Ljava/lang/String;)Ljava/io/File;");
        jobject jFile = env->CallObjectMethod(gApplicationContext, methodID, nullptr);
        if(jFile)
        {
            ret = pathJoin(java_File_getAbsolutePath(env, jFile), path);
            env->DeleteLocalRef(jFile);
        }
        env->DeleteLocalRef(applicationContextClass);
    }
    return ret;
}

void Platform::sysCall(int32_t id, const String& value)
{
    DASSERT(gApplicationContext);
    JNIEnv* env = JNIUtil::attachCurrentThread();
    jclass applicationContextClass = env->GetObjectClass(gApplicationContext);
    jmethodID methodID = env->GetMethodID(applicationContextClass, "sysCall", "(ILjava/lang/String;)V");
    jstring jValue = JNIUtil::newStringUTF(env, value.c_str());
    env->CallVoidMethod(gApplicationContext, methodID, id, jValue);
    env->DeleteLocalRef(jValue);
    env->DeleteLocalRef(applicationContextClass);
}

String Platform::getRealPath(const String& path)
{
    return path;
}

String Platform::getDefaultFontDirectory()
{
    return "/system/fonts";
}

sp<Alphabet> Platform::getSystemAlphabet(const Font& font, const String& lang)
{
	String alphabet = "DroidSans.ttf";
	String fallback = "DroidSansFallback.ttf";
	_loadSystemFontWithFallback(font, lang, alphabet, fallback);
	bool hasFallback = Platform::isFile("/system/fonts/" + fallback);
	LOGD("Default font: %s. HasFallback: %d. Fallback font %s", alphabet.c_str(), hasFallback, fallback.c_str());
	const sp<Alphabet> sa = sp<AlphabetTrueType>::make(alphabet, font.size());
    return hasFallback ? sp<Alphabet>::adopt(new AlphabetWithFallback(sa, sp<AlphabetTrueType>::make(fallback, font.size()))) : sa;
}

String Platform::getExecutablePath()
{
    return "";
}

String Platform::glShaderVersionDeclaration(uint32_t versionCode)
{
	uint32_t version = 0;
	if(versionCode == 130)
		version = 300;
	else if(versionCode >= 140)
		version = 310;

    return version ? Strings::sprintf("#version %d es\n#extension GL_EXT_clip_cull_distance : require\n", version) : "";
}

uint32_t Platform::glPreprocessShader(const String& shader, const char* srcs[], uint32_t length)
{
    DCHECK(length > 1, "length = %d", length);
    srcs[0] = "precision highp float;\n";
    srcs[1] = shader.c_str();
    return 2;
}

void* Platform::dlOpen(const char* name)
{
    if(!name)
        return dlopen(nullptr, RTLD_LAZY);

    String soName = Strings::sprintf("lib%s.so", name);
    return dlopen(soName.c_str(), RTLD_LAZY);
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
	JNIUtil::detachFromVM();
}

char PlatformIndependent::_DIR_SEPARATOR = '/';
char PlatformIndependent::_PATH_SEPARATOR = ':';

}
