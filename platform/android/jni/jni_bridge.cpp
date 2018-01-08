
#include <jni.h>

#include "core/ark.h"
#include "core/dom/document.h"
#include "core/types/shared_ptr.h"

#include "graphics/base/size.h"

#include "util/jni_util.h"

#include "app/base/application_delegate_impl.h"
#include "app/base/application_manifest.h"
#include "app/base/event.h"


#include "impl/application/android_application.h"

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
    const document appManifest = _ark->manifest()->getChild("application");
    NOT_NULL(appManifest);
    const sp<ApplicationManifest> applicationManifest = sp<ApplicationManifest>::make(appManifest);
    const sp<Size>& renderResolution = applicationManifest->renderResolution();
    Viewport viewport(0.0f, 0.0f, renderResolution->width(), renderResolution->height(), 0.0f, renderResolution->width());
    _application = sp<AndroidApplication>::make(sp<ApplicationDelegateImpl>::make(applicationManifest), _ark->applicationContext(), (int32_t) (renderResolution->width()), (int32_t) (renderResolution->height()), viewport);
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
    return _application->onEvent(Event(s, x, y, timestamp));
}
