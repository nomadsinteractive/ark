#ifndef ARK_CORE_COLLECTION_BY_INDEX_H_
#define ARK_CORE_COLLECTION_BY_INDEX_H_

#include <unordered_map>

#include "core/base/api.h"
#include "core/util/strings.h"

namespace ark {

template<typename T> class ByIndex {
public:
    ByIndex() = default;
    DEFAULT_COPY_AND_ASSIGN(ByIndex);

    void add(int32_t index, const T& item) {
        _items[index] = item;
    }

    void update(int32_t index, const T& item) {
        at(index) = item;
    }

    template<typename... Args> void emplace(int32_t index, Args&&... args) {
        _items[index] = T(std::forward<Args>(args)...);
    }

    std::unordered_map<int32_t, T>& items() {
        return _items;
    }

    const std::unordered_map<int32_t, T>& items() const {
        return _items;
    }

    bool has(int32_t index) const {
        return _items.find(index) != _items.end();
    }

    const T& at(int32_t index) const {
        auto iter = _items.find(index);
        DCHECK(iter != _items.end(), "Item with index '%d' doesn't exists", index);
        return iter->second;
    }

    T& at(int32_t index) {
        auto iter = _items.find(index);
        DCHECK(iter != _items.end(), "Item with index '%d' doesn't exists", index);
        return iter->second;
    }

private:
    std::unordered_map<int32_t, T> _items;
};

}

#endif
