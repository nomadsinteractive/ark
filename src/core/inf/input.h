#ifndef ARK_CORE_INF_INPUT_H_
#define ARK_CORE_INF_INPUT_H_

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/inf/updatable.h"

namespace ark {

class ARK_API Input : public Updatable {
public:
    Input(size_t size)
        : _size(size) {
    }
    virtual ~Input() override = default;

    virtual void upload(Writable& buf) = 0;

    virtual bool update(uint64_t /*timestamp*/) override {
        return false;
    }

    size_t size() const {
        return _size;
    }

protected:
    size_t _size;

};

}

#endif
