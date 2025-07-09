#pragma once

#include "core/base/api.h"
#include "core/base/string.h"
#include "core/forwarding.h"

#include "app/base/application_delegate.h"
#include "app/forwarding.h"

namespace ark {

class ARK_API ApplicationDelegateImpl final : public ApplicationDelegate {
public:
    ApplicationDelegateImpl() = default;

    void onCreate(Application& application, const sp<Surface>& surface) override;
    void onPause() override;
    void onResume() override;

    enum ScriptRunOn {
        SCRIPT_RUN_ON_CREATE,
        SCRIPT_RUN_ON_PAUSE,
        SCRIPT_RUN_ON_RESUME,
        SCRIPT_RUN_ON_EVENT,
        SCRIPT_RUN_ON_UNHANDLED_EVENT
    };

private:
    struct ScriptTag {
        ScriptTag(sp<Interpreter> interpreter, const document& manifest);

        void run() const;

        sp<EventListener> makeEventListener() const;

        ScriptRunOn _on;
        String _function_name;
        sp<Asset> _source;

        sp<Interpreter> _interpreter;
    };

private:
    Vector<ScriptTag> _interpreter;
};

}
