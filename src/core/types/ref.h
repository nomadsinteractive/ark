#pragma once

#include "core/base/api.h"

namespace ark {

template<typename T> class Ref {
public:
    Ref(T& instance)
        : _instance(instance), _discarded(false) {
    }

    void discard() {
        _discarded = true;
    }

    T& instance() {
        ASSERT(!_discarded);
        return _instance;
    }

private:
    T& _instance;
    bool _discarded;
};

}
