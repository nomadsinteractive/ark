#ifndef ARK_CORE_IMPL_UPDATABLE_UPDATABLE_COMPOSITE_H_
#define ARK_CORE_IMPL_UPDATABLE_UPDATABLE_COMPOSITE_H_

#include <vector>

#include "core/inf/updatable.h"
#include "core/types/shared_ptr.h"

namespace ark {

class UpdatableComposite : public Updatable {
public:
    UpdatableComposite(std::vector<sp<Updatable>> updatables);

    virtual bool update(uint64_t timestamp) override;

private:
    std::vector<sp<Updatable>> _updatables;

};

}

#endif
