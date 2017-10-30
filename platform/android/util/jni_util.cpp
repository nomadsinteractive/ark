#include "util/jni_util.h"

#include "core/util/strings.h"

namespace ark {

JavaVM* JNIUtil::g_jvm = nullptr;

void JNIUtil::init(JNIEnv* env)
{
    env->GetJavaVM(&g_jvm);
}

JNIEnv* JNIUtil::attachCurrentThread()
{
    if(!g_jvm)
        return nullptr;
    JNIEnv** env_ptr = getCurrentJNIEnv();
    if(*env_ptr == nullptr)
    {
        jint ret = g_jvm->AttachCurrentThread(env_ptr, nullptr);
        DCHECK(ret == JNI_OK, "JNI AttachCurrentThread failed.");
    }
    return *env_ptr;
}

void JNIUtil::detachFromVM()
{
    JNIEnv** env_ptr = getCurrentJNIEnv();
    if(g_jvm && *env_ptr)
    {
        g_jvm->DetachCurrentThread();
        *env_ptr = nullptr;
    }
}

jclass JNIUtil::findClass(JNIEnv* env, const char* name)
{
    jclass clazz = env->FindClass(name);
    DCHECK(clazz, "\"%s\" Class not found", name);
    return clazz;
}

jstring JNIUtil::newStringUTF(JNIEnv* env, const char* str)
{
    return env->NewStringUTF(str);
}

JNIEnv** JNIUtil::getCurrentJNIEnv()
{
    thread_local JNIEnv* env = nullptr;
    return &env;
}

}
