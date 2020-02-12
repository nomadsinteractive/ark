#ifndef ARK_CORE_INF_UPDATABLE_H_
#define ARK_CORE_INF_UPDATABLE_H_

#include <stdint.h>
#include <limits>

#include "core/base/api.h"

namespace ark {

class ARK_API Updatable {
public:
    virtual ~Updatable() = default;

    virtual bool update(uint64_t timestamp) = 0;

    class Timestamp;

};


class Updatable::Timestamp : public Updatable {
public:
    Timestamp(bool dirty = false)
        : _timestamp(dirty ? std::numeric_limits<uint64_t>::max() : 0) {
    }

    virtual bool update(uint64_t timestamp) override {
        if(_timestamp >= timestamp) {
            _timestamp = timestamp;
            return true;
        }
        return false;
    }

    void setDirty() {
        _timestamp = std::numeric_limits<uint64_t>::max();
    }

private:
    uint64_t _timestamp;
};

}

#endif
