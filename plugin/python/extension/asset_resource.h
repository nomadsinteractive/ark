#ifndef ARK_PLUGIN_PYTHON_EXTENSION_ASSET_RESOURCE_H_
#define ARK_PLUGIN_PYTHON_EXTENSION_ASSET_RESOURCE_H_

#include "core/types/shared_ptr.h"
#include "core/forwarding.h"

namespace ark {
namespace plugin {
namespace python {

class AssetResource {
public:
    AssetResource(const sp<AssetBundle>& resource);

//  [[script::bindings::auto]]
    sp<String> getString(const String& filepath);
//  [[script::bindings::auto]]
    String getRealPath(const String& filepath);

private:
    sp<AssetBundle> _asset;
};

}
}
}

#endif
