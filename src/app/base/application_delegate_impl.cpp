#include "app/base/application_delegate_impl.h"

#include "core/base/scope.h"
#include "core/base/string_table.h"
#include "core/inf/script.h"
#include "core/types/global.h"
#include "core/util/log.h"

#include "app/base/application.h"
#include "app/base/application_context.h"
#include "app/base/application_manifest.h"
#include "app/base/resource_loader.h"
#include "app/base/surface.h"
#include "app/impl/event_listener/event_listener_by_script.h"

namespace ark {

ApplicationDelegateImpl::ApplicationDelegateImpl(const sp<ApplicationManifest>& applicationManifest)
    : _application_manifest(applicationManifest)
{
}

const char* ApplicationDelegateImpl::name()
{
    return _application_manifest->name().c_str();
}

void ApplicationDelegateImpl::onCreate(ark::Application& application, const sp<ark::Surface>& surface)
{
    LOGD("onCreate");
    ApplicationDelegate::onCreate(application, surface);

    const sp<ResourceLoader> appResourceLoader = application.context()->createResourceLoader("application.xml");
    NOT_NULL(appResourceLoader);
    _script = appResourceLoader->load<Script>("script");
    NOT_NULL(_script);

    const sp<Scope> vars = sp<Scope>::make();
    vars->put<ApplicationContext>("_application_context", application.context());
    vars->put<ResourceLoader>("_resource_loader", appResourceLoader);
    vars->put<SurfaceController>("_surface_controller", surface->controller());
    vars->put<Size>("_render_resolution", _application_manifest->renderResolution());

    application.context()->setDefaultEventListener(sp<EventListenerByScript>::make(_script, "on_unhandled_event"));

    for(const document& i : _application_manifest->manifest()->children("script"))
    {
        const String& src = Documents::ensureAttribute(i, Constants::Attributes::SRC);
        const sp<Readable> readable = Ark::instance().getResource(src);
        DCHECK(readable, "Cannot open script \"%s\"", src.c_str());
        _script->run(Strings::loadFromReadable(readable), vars);
    }
}

void ApplicationDelegateImpl::onPause()
{
    const Script::Arguments args;
    _script->call("on_pause", args);
}

void ApplicationDelegateImpl::onResume()
{
    const Script::Arguments args;
    _script->call("on_resume", args);
}

}
