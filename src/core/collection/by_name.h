#ifndef ARK_CORE_COLLECTION_BY_NAME_H_
#define ARK_CORE_COLLECTION_BY_NAME_H_

#include <map>

#include "core/base/api.h"
#include "core/base/string.h"
#include "core/types/box.h"
#include "core/types/null.h"
#include "core/types/shared_ptr.h"

namespace ark {

class ARK_API ByName {
public:
    ByName() = default;
    ByName(const ByName& other) = default;
    ByName(ByName&& other) = default;

    ByName& operator =(const ByName& other) = default;
    ByName& operator =(ByName&& other) = default;

    template<typename T> void put(const String& name, const sp<T>& item) {
        _items[name] = item.pack();
    }

    template<typename T> const sp<T>& get(const String& name) const {
        auto iter = _items.find(name);
        return iter != _items.end() ? iter->second.template unpack<T>() : sp<T>::null();
    }

private:
    std::map<String, Box> _items;
};

}

#endif
