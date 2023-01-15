#ifndef ARK_CORE_IMPL_UPDATABLE_UPDATABLE_WRAPPER_H_
#define ARK_CORE_IMPL_UPDATABLE_UPDATABLE_WRAPPER_H_

#include "core/base/timestamp.h"
#include "core/base/wrapper.h"
#include "core/inf/updatable.h"

namespace ark {

class UpdatableWrapper : public Updatable, public Wrapper<Updatable> {
public:
    UpdatableWrapper(sp<Updatable> updatable);

    virtual bool update(uint64_t timestamp) override;

    void reset(sp<Updatable> delegate);

private:
    Timestamp _timestamp;
};

}

#endif
