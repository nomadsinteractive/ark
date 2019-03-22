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
        _indices[index] = item;
    }

    void update(int32_t index, const T& item) {
        at(index) = item;
    }

    template<typename... Args> void emplace(int32_t index, Args&&... args) {
        _indices[index] = T(std::forward<Args>(args)...);
    }

    std::unordered_map<int32_t, T>& indices() {
        return _indices;
    }

    const std::unordered_map<int32_t, T>& indices() const {
        return _indices;
    }

    bool has(int32_t index) const {
        return _indices.find(index) != _indices.end();
    }

    const T& at(int32_t index) const {
        auto iter = _indices.find(index);
        DCHECK(iter != _indices.end(), "Item with index '%d' doesn't exists", index);
        return iter->second;
    }

    T& at(int32_t index) {
        auto iter = _indices.find(index);
        DCHECK(iter != _indices.end(), "Item with index '%d' doesn't exists", index);
        return iter->second;
    }

private:
    std::unordered_map<int32_t, T> _indices;
};

}

#endif
