#ifndef ARK_APP_BASE_APPLICATION_DELEGATE_IMPL_H_
#define ARK_APP_BASE_APPLICATION_DELEGATE_IMPL_H_

#include "core/base/api.h"
#include "core/forwarding.h"

#include "graphics/forwarding.h"

#include "app/base/application_delegate.h"
#include "app/forwarding.h"

namespace ark {

class ARK_API ApplicationDelegateImpl : public ApplicationDelegate {
public:
    ApplicationDelegateImpl(const sp<ApplicationManifest>& applicationManifest);

    virtual const char* name() override;
    virtual void onCreate(Application& application, const sp<Surface>& surface) override;

    virtual void onPause() override;
    virtual void onResume() override;

private:
    sp<ApplicationManifest> _application_manifest;
    sp<Script> _script;
};

}

#endif
