#pragma once

#include <vector>

#include "core/types/shared_ptr.h"

namespace ark {

template<typename T> class ChildNodes {
public:
    ChildNodes(std::vector<sp<T>> nodes = {})
        : _nodes(std::move(nodes)) {
    }

    const std::vector<sp<T>>& nodes() const {
        return _nodes;
    }

    std::vector<sp<T>>& nodes() {
        return _nodes;
    }

private:
    std::vector<sp<T>> _nodes;
};

}
