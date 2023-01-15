#ifndef ARK_CORE_BASE_TIMESTAMP_H_
#define ARK_CORE_BASE_TIMESTAMP_H_

#include "core/inf/updatable.h"


namespace ark {

class ARK_API Timestamp : public Updatable {
public:
    Timestamp();

    virtual bool update(uint64_t timestamp) override;

    void markDirty();

private:
    uint64_t _last_modified;
};

}
#endif
