#ifndef ARK_CORE_UTIL_STORAGE_TYPE_H_
#define ARK_CORE_UTIL_STORAGE_TYPE_H_

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/base/json.h"

namespace ark {

//[[script::bindings::class("Storage")]]
class ARK_API StorageUtil {
public:

//[[script::bindings::classmethod]]
    static void load(const sp<Storage>& self, const sp<Readable>& src);
//[[script::bindings::classmethod]]
    static void save(const sp<Storage>& self, const sp<Writable>& out);

//[[script::bindings::classmethod]]
    static void jsonLoad(const sp<Storage>& self, const Json& json);
//[[script::bindings::classmethod]]
    static Json jsonDump(const sp<Storage>& self);

};

}

#endif
