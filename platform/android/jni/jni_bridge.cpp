
#include <jni.h>

#include "core/ark.h"
#include "core/dom/document.h"
#include "core/types/shared_ptr.h"

#include "platform/platform.h"

#include "util/jni_util.h"

#include "app/base/event.h"
#include "app/impl/application_delegate/application_delegate_impl.h"

#include "impl/application/android_application.h"

#include "app/constants.h"

extern "C" {
    JNIEXPORT void JNICALL Java_com_nomads_ark_JNILib_onCreate(JNIEnv* env, jobject obj, jobject applicationContext, jobject assetManager);
    JNIEXPORT void JNICALL Java_com_nomads_ark_JNILib_onSurfaceCreated(JNIEnv* env, jobject obj);
    JNIEXPORT void JNICALL Java_com_nomads_ark_JNILib_onSurfaceChanged(JNIEnv* env, jobject obj, jint width, jint height);
    JNIEXPORT void JNICALL Java_com_nomads_ark_JNILib_onDraw(JNIEnv* env, jobject obj);
    JNIEXPORT void JNICALL Java_com_nomads_ark_JNILib_onPause(JNIEnv* env, jobject obj);
    JNIEXPORT void JNICALL Java_com_nomads_ark_JNILib_onResume(JNIEnv* env, jobject obj);
    JNIEXPORT void JNICALL Java_com_nomads_ark_JNILib_onDestroy(JNIEnv* env, jobject obj);
    JNIEXPORT jboolean JNICALL Java_com_nomads_ark_JNILib_onEvent(JNIEnv* env, jobject obj, jint action, jfloat x, jfloat y, jlong timestamp);
};

namespace ark {
namespace platform {
namespace android {
	
ARK_API extern jobject gApplicationContext;
ARK_API extern jobject gAssetManager;

}
}
}

using namespace ark;
using namespace ark::platform::android;

static sp<Ark> _ark;
static sp<Application> _application;


JNIEXPORT void JNICALL Java_com_nomads_ark_JNILib_onCreate(JNIEnv* env, jobject obj, jobject applicationContext, jobject assetManager)
{
	JNIUtil::init(env);
	if(gApplicationContext)
		env->DeleteGlobalRef(gApplicationContext);
	gApplicationContext = env->NewGlobalRef(applicationContext);
	if(gAssetManager)
		env->DeleteGlobalRef(gAssetManager);
	gAssetManager = env->NewGlobalRef(assetManager);
	
	_ark = sp<Ark>::make(0, nullptr, "manifest.xml");
	Viewport viewport(0.0f, 0.0f, SCREEN_RESOLUTION->width(), SCREEN_RESOLUTION->height(), 0.0f, SCREEN_RESOLUTION->width());
	_application = sp<AndroidApplication>::make(sp<ApplicationDelegateImpl>::make(), _ark->applicationContext(), (int32_t) (SCREEN_RESOLUTION->width()), (int32_t) (SCREEN_RESOLUTION->height()), viewport);
	_application->onCreate();
}	

JNIEXPORT void JNICALL Java_com_nomads_ark_JNILib_onSurfaceCreated(JNIEnv* env, jobject obj)
{
	_application->onSurfaceCreated();
}

JNIEXPORT void JNICALL Java_com_nomads_ark_JNILib_onSurfaceChanged(JNIEnv* env, jobject obj, jint width, jint height)
{
	_application->onSurfaceChanged(width, height);
}

JNIEXPORT void JNICALL Java_com_nomads_ark_JNILib_onDraw(JNIEnv* env, jobject obj)
{
	_application->onSurfaceDraw();
}

JNIEXPORT void JNICALL Java_com_nomads_ark_JNILib_onPause(JNIEnv* env, jobject obj)
{
	_application->onPause();
}

JNIEXPORT void JNICALL Java_com_nomads_ark_JNILib_onResume(JNIEnv* env, jobject obj)
{
	_application->onResume();
}

JNIEXPORT void JNICALL Java_com_nomads_ark_JNILib_onDestroy(JNIEnv* env, jobject obj)
{
	_application->onDestroy();
	_application = nullptr;
	_ark = nullptr;
}

JNIEXPORT jboolean JNICALL Java_com_nomads_ark_JNILib_onEvent(JNIEnv* env, jobject obj, jint action, jfloat x, jfloat y, jlong timestamp)
{
	return _application->onEvent(Event(action == 0 ? Event::ACTION_DOWN : (action == 1 ? Event::ACTION_UP : Event::ACTION_MOVE), x, y, timestamp));
}
