#ifndef ARK_APP_BASE_APPLICATION_DELEGATE_IMPL_H_
#define ARK_APP_BASE_APPLICATION_DELEGATE_IMPL_H_

#include <list>

#include "core/base/api.h"
#include "core/base/scope.h"
#include "core/base/string.h"
#include "core/forwarding.h"

#include "graphics/forwarding.h"

#include "app/base/application_delegate.h"
#include "app/forwarding.h"

namespace ark {

class ARK_API ApplicationDelegateImpl : public ApplicationDelegate {
public:
    ApplicationDelegateImpl(const sp<ApplicationManifest>& manifest);

    virtual void onCreate(Application& application, const sp<Surface>& surface) override;

    virtual void onPause() override;
    virtual void onResume() override;

    enum ScriptRunOn {
        SCRIPT_RUN_ON_CREATE,
        SCRIPT_RUN_ON_PAUSE,
        SCRIPT_RUN_ON_RESUME,
        SCRIPT_RUN_ON_EVENT,
        SCRIPT_RUN_ON_UNHANDLED_EVENT
    };

private:
    struct ScriptTag {
        ScriptTag(ResourceLoader& resourceLoader, const document& manifest, const sp<Scope>& vars);

        void run() const;

        sp<EventListener> makeEventListener() const;

        ScriptRunOn _on;
        String _function_name;
        sp<Asset> _source;

        sp<Script> _script;
        sp<Scope> _vars;
    };

private:
    std::list<ScriptTag> _scripts;

};

}

#endif
