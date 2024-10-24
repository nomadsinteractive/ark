#include "app/base/application_delegate_impl.h"

#include "core/ark.h"
#include "core/base/scope.h"
#include "core/base/resource_loader.h"
#include "core/inf/interpreter.h"
#include "core/util/string_convert.h"
#include "core/util/log.h"

#include "app/base/application.h"
#include "app/base/application_context.h"
#include "app/base/application_facade.h"
#include "app/base/application_manifest.h"
#include "app/base/surface.h"
#include "app/impl/event_listener/event_listener_by_script.h"
#include "app/view/arena.h"

namespace ark {

ApplicationDelegateImpl::ApplicationDelegateImpl(const sp<ApplicationManifest>& manifest)
    : ApplicationDelegate(manifest)
{
}

void ApplicationDelegateImpl::onCreate(Application& application, const sp<Surface>& surface)
{
    LOGD("onCreate");
    ApplicationDelegate::onCreate(application, surface);

    const sp<ApplicationContext>& applicationContext = application.context();
    const document& appManifest = _manifest->content()->getChild("application");
    DCHECK(appManifest, "Manifest has no <application/> node");
    const sp<ResourceLoader>& appResourceLoader = applicationContext->resourceLoader();
    DASSERT(appResourceLoader);

    const sp<Scope> vars = sp<Scope>::make();
    const sp<ApplicationFacade> applicationFacade = sp<ApplicationFacade>::make(application, surface, _manifest);
    vars->put("_application", applicationFacade);

    applicationFacade->setBackgroundColor(Documents::getAttribute<Color>(appManifest, "background-color", Color(0, 0, 0)));

    if(sp<Arena> arena = appResourceLoader->beanFactory().build<Arena>(appManifest, "arena", {}))
        applicationFacade->setArena(std::move(arena));

    bool defaultEventListenerSet = false;
    for(const document& i : appManifest->children("script"))
    {
        ScriptTag script(_application_context->interpreter(), i, vars);
        if(script._on == SCRIPT_RUN_ON_CREATE)
            script.run();
        else if(script._on == SCRIPT_RUN_ON_EVENT)
            applicationContext->addEventListener(script.makeEventListener());
        else if(script._on == SCRIPT_RUN_ON_UNHANDLED_EVENT)
        {
            DCHECK_WARN(!defaultEventListenerSet, "Default EventListener has been set already");
            applicationContext->setDefaultEventListener(script.makeEventListener());
            defaultEventListenerSet = true;
        }
        else
            _interpreter.push_back(std::move(script));
    }
}

void ApplicationDelegateImpl::onPause()
{
    for(const auto& i: _interpreter)
        if(i._on == SCRIPT_RUN_ON_PAUSE)
            i.run();
}

void ApplicationDelegateImpl::onResume()
{
    for(const auto& i: _interpreter)
        if(i._on == SCRIPT_RUN_ON_RESUME)
            i.run();
}

ApplicationDelegateImpl::ScriptTag::ScriptTag(sp<Interpreter> interpreter, const document& manifest, sp<Scope> vars)
    : _on(Documents::getAttribute(manifest, "on", SCRIPT_RUN_ON_CREATE)),
      _function_name(Documents::getAttribute(manifest, "function")),
      _interpreter(std::move(interpreter)), _vars(std::move(vars))
{
    if(const String src = Documents::getAttribute(manifest, constants::SRC))
    {
        _source = Ark::instance().getAsset(src);
        CHECK(_source, "Cannot open script \"%s\"", src.c_str());
    }
}

void ApplicationDelegateImpl::ScriptTag::run() const
{
    DASSERT(_interpreter);
    if(_source)
        _interpreter->execute(_source, _vars);
    if(_function_name)
    {
        const Interpreter::Arguments args;
        _interpreter->call(_interpreter->attr(nullptr, _function_name), args);
    }
}

sp<EventListener> ApplicationDelegateImpl::ScriptTag::makeEventListener() const
{
    DCHECK(_function_name, "Application EventListener should be function, not script");
    return sp<EventListenerByScript>::make(_interpreter, _function_name);
}

template<> ApplicationDelegateImpl::ScriptRunOn StringConvert::eval<ApplicationDelegateImpl::ScriptRunOn>(const String& str)
{
    if(str == "create")
        return ApplicationDelegateImpl::SCRIPT_RUN_ON_CREATE;
    if(str == "pause")
        return ApplicationDelegateImpl::SCRIPT_RUN_ON_PAUSE;
    if(str == "resume")
        return ApplicationDelegateImpl::SCRIPT_RUN_ON_RESUME;
    if(str == "event")
        return ApplicationDelegateImpl::SCRIPT_RUN_ON_EVENT;
    if(str == "unhandled_event")
        return ApplicationDelegateImpl::SCRIPT_RUN_ON_UNHANDLED_EVENT;
    DFATAL("Unkown value %s", str.c_str());
    return ApplicationDelegateImpl::SCRIPT_RUN_ON_CREATE;
}

}
