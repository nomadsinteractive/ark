#include "app/impl/application/application_delegate_impl.h"

#include "core/ark.h"
#include "core/base/scope.h"
#include "core/base/resource_loader.h"
#include "core/inf/interpreter.h"
#include "core/util/log.h"

#include "app/base/application.h"
#include "app/base/application_context.h"
#include "app/base/application_facade.h"
#include "app/base/application_manifest.h"
#include "app/base/surface.h"
#include "app/base/activity.h"

namespace ark {

void ApplicationDelegateImpl::onCreate(Application& application, const sp<Surface>& surface)
{
    LOGD("onCreate");
    ApplicationDelegate::onCreate(application, surface);

    const sp<ApplicationContext>& applicationContext = application.context();
    const document& appManifest = Ark::instance().manifest()->content()->getChild("application");
    DCHECK(appManifest, "Manifest has no <application/> node");
    const sp<ResourceLoader>& appResourceLoader = applicationContext->resourceLoader();
    DASSERT(appResourceLoader);

    sp<ApplicationFacade> applicationFacade = sp<ApplicationFacade>::make(application, surface);
    applicationFacade->setBackgroundColor(Documents::getAttribute<Color>(appManifest, "background-color", Color(0, 0, 0)));

    const SafeBuilder<Activity> activityBuilder(appResourceLoader->beanFactory().getBuilder<Activity>(appManifest, "activity"));
    if(sp<Activity> activity = activityBuilder.build({}))
        applicationFacade->setActivity(std::move(activity));

    applicationContext->_application_facade = std::move(applicationFacade);

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

}
