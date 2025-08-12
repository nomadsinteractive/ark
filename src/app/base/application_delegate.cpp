#include "app/base/application_delegate.h"

#include "core/ark.h"
#include "core/base/resource_loader.h"
#include "core/base/scope.h"
#include "core/inf/interpreter.h"
#include "core/util/log.h"

#include "app/base/activity.h"
#include "app/base/application.h"
#include "app/base/application_context.h"
#include "app/base/application_facade.h"
#include "app/base/application_manifest.h"
#include "app/base/surface.h"
#include "app/inf/application_event_listener.h"

namespace ark {

void ApplicationDelegate::onCreate(Application& application, const sp<Surface>& surface)
{
    _application_context = application.context();
    _application_context->interpreter()->initialize();
    _surface = surface;

    const document& appManifest = Ark::instance().manifest()->content()->getChild("application");
    DCHECK(appManifest, "Manifest has no <application/> node");
    const sp<ResourceLoader>& appResourceLoader = _application_context->resourceLoader();
    DASSERT(appResourceLoader);

    sp<ApplicationFacade> applicationFacade = sp<ApplicationFacade>::make(application, surface);
    applicationFacade->setBackgroundColor(Documents::getAttribute<V4>(appManifest, "background-color", V4(0, 0, 0, 1.0f)));

    const SafeBuilder<Activity> activityBuilder(appResourceLoader->beanFactory().getBuilder<Activity>(appManifest, "activity"));
    if(sp<Activity> activity = activityBuilder.build({}))
        applicationFacade->setActivity(std::move(activity));

    _application_context->_application_facade = std::move(applicationFacade);

    const sp<Interpreter>& interpreter = _application_context->interpreter();
    for(const document& i : appManifest->children("script"))
    {
        if(const String src = Documents::getAttribute(i, constants::SRC))
        {
            const sp<Asset> sourceAsset = Ark::instance().getAsset(src);
            CHECK(sourceAsset, "Cannot open script \"%s\"", src.c_str());
            interpreter->execute(sourceAsset);
        }
        if(const String func = Documents::getAttribute(i, "function"))
            interpreter->call(interpreter->attr(nullptr, func), {});
    }
}

void ApplicationDelegate::onPause()
{
    if(_application_context->applicationEventListener())
        _application_context->applicationEventListener()->onPause();
}

void ApplicationDelegate::onResume()
{
    if(_application_context->applicationEventListener())
        _application_context->applicationEventListener()->onResume();
}

void ApplicationDelegate::onDestroy()
{
    _render_view = nullptr;
    _application_context = nullptr;
}

void ApplicationDelegate::onSurfaceCreated(const sp<Surface>& surface)
{
    _render_view = surface->renderView();
}

void ApplicationDelegate::onSurfaceChanged(uint32_t /*width*/, uint32_t /*height*/)
{
}

void ApplicationDelegate::onSurfaceDraw()
{
    _surface->onRenderFrame(_application_context->backgroundColor());
}

bool ApplicationDelegate::onEvent(const Event& event)
{
    return _application_context->onEvent(event);
}

}
