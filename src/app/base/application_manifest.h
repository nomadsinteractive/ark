#ifndef ARK_APP_BASE_APPLICATION_MANIFEST_H_
#define ARK_APP_BASE_APPLICATION_MANIFEST_H_

#include "core/base/api.h"
#include "core/base/string.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "app/forwarding.h"

namespace ark {

class ARK_API ApplicationManifest {
public:
    ApplicationManifest(const document& manifest);

// [[script::bindings::property]]
    const String& name() const;
// [[script::bindings::property]]
    const sp<Size>& renderResolution() const;

    const document& manifest() const;

private:
    String _name;
    sp<Size> _render_resolution;

    document _manifest;
};

}

#endif
