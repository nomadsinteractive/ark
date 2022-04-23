#ifndef ARK_CORE_UTIL_STORAGE_TYPE_H_
#define ARK_CORE_UTIL_STORAGE_TYPE_H_

#include "core/base/api.h"
#include "core/forwarding.h"

namespace ark {

//[[script::bindings::class("Storage")]]
class ARK_API StorageUtil {
public:

//[[script::bindings::classmethod]]
    static void load(const sp<Storage>& self, const sp<Readable>& src);
//[[script::bindings::classmethod]]
    static void save(const sp<Storage>& self, const sp<Writable>& out);

};

}

#endif
