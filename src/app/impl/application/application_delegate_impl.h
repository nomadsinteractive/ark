#pragma once

#include "core/base/api.h"
#include "core/forwarding.h"

#include "app/base/application_delegate.h"
#include "app/forwarding.h"

namespace ark {

class ARK_API ApplicationDelegateImpl final : public ApplicationDelegate {
public:
    ApplicationDelegateImpl() = default;

    void onCreate(Application& application, const sp<Surface>& surface) override;
};

}
