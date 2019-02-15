
#include <jni.h>

#include <android/native_window.h>
#include <android/native_window_jni.h> 

#include "core/ark.h"
#include "core/base/manifest.h"
#include "core/types/shared_ptr.h"
#include "core/util/log.h"

#include "graphics/base/size.h"

#include "renderer/base/render_engine.h"
#include "renderer/base/render_context.h"

#include "app/base/application_context.h"
#include "app/base/application_delegate_impl.h"
#include "app/base/event.h"

#include "impl/application/android_application.h"
#include "util/jni_util.h"

extern "C" {
    JNIEXPORT void JNICALL Java_com_nomads_ark_JNILib_onCreate(JNIEnv* env, jobject obj, jobject applicationContext, jobject assetManager);
    JNIEXPORT void JNICALL Java_com_nomads_ark_JNILib_onSurfaceCreated(JNIEnv* env, jobject obj, jobject surface);
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

static ANativeWindow* _window;


JNIEXPORT void JNICALL Java_com_nomads_ark_JNILib_onCreate(JNIEnv* env, jobject obj, jobject applicationContext, jobject assetManager)
{
    JNIUtil::init(env);
    if(gApplicationContext)
        env->DeleteGlobalRef(gApplicationContext);
    gApplicationContext = env->NewGlobalRef(applicationContext);
    if(gAssetManager)
        env->DeleteGlobalRef(gAssetManager);
    gAssetManager = env->NewGlobalRef(assetManager);
    
	const sp<Manifest> manifest = sp<Manifest>::make("manifest.xml");
    _ark = sp<Ark>::make(0, nullptr, manifest);
    const sp<Size>& renderResolution = manifest->rendererResolution();
    Viewport viewport(0.0f, 0.0f, renderResolution->width(), renderResolution->height(), 0.0f, renderResolution->width());
    _application = sp<AndroidApplication>::make(sp<ApplicationDelegateImpl>::make(manifest), _ark->applicationContext(), (int32_t) (renderResolution->width()), (int32_t) (renderResolution->height()), viewport);
    _application->onCreate();
}

JNIEXPORT void JNICALL Java_com_nomads_ark_JNILib_onSurfaceCreated(JNIEnv* env, jobject obj, jobject surface)
{
	RenderContext::Info& info = _application->context()->renderEngine()->renderContext()->info();
	_window = ANativeWindow_fromSurface(env, surface);
	info.android.window = _window;
    _application->onSurfaceCreated();
	_ark->applicationContext()->updateRenderState();
}

JNIEXPORT void JNICALL Java_com_nomads_ark_JNILib_onSurfaceChanged(JNIEnv* env, jobject obj, jint width, jint height)
{
    _application->onSurfaceChanged(width, height);
}

JNIEXPORT void JNICALL Java_com_nomads_ark_JNILib_onDraw(JNIEnv* env, jobject obj)
{
    _application->onSurfaceUpdate();
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
	ANativeWindow_release(_window);
    _application = nullptr;
    _ark = nullptr;
	_window = nullptr;
}

JNIEXPORT jboolean JNICALL Java_com_nomads_ark_JNILib_onEvent(JNIEnv* env, jobject obj, jint action, jfloat x, jfloat y, jlong timestamp)
{
    Event::Action s = Event::ACTION_KEY_NONE;
    switch(action)
    {
        case 0:
            s = Event::ACTION_DOWN;
            break;
        case 1:
            s = Event::ACTION_UP;
            break;
        case 2:
            s = Event::ACTION_MOVE;
            break;
        case 3:
            s = Event::ACTION_CANCEL;
            break;
        case 10000:
            s = Event::ACTION_BACK_PRESSED;
            break;
        default:
            DFATAL("Unrecognized action code: %d", action);
            break;
    }
    return _application->onEvent(Event(s, x, y, timestamp), true);
}
