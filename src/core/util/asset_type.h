#ifndef ARK_CORE_UTIL_ASSET_TYPE_H_
#define ARK_CORE_UTIL_ASSET_TYPE_H_

#include "core/forwarding.h"
#include "core/base/string.h"

namespace ark {

//[[script::bindings::class("Asset")]]
class ARK_API AssetType final {
public:

//  [[script::bindings::property]]
    static String location(const sp<Asset>& self);

//  [[script::bindings::classmethod]]
    static String readString(const sp<Asset>& self);

};

}

#endif
