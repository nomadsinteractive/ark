#ifndef ARK_PLATFORM_ANDROID_UTIL_JNI_UTIL_H_
#define ARK_PLATFORM_ANDROID_UTIL_JNI_UTIL_H_

#include <jni.h>

#include "core/base/api.h"

namespace ark {

class ARK_API JNIUtil {
public:
	static void init(JNIEnv* env);
	static JNIEnv* attachCurrentThread();
	static void detachFromVM();
	
	static jclass findClass(JNIEnv* env, const char* name);
	static jstring newStringUTF(JNIEnv* env, const char* str);
	
private:
	static JNIEnv** getCurrentJNIEnv();

private:
	static JavaVM* g_jvm;
};

}

#endif
