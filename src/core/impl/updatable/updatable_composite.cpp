#include "core/impl/updatable/updatable_composite.h"

namespace ark {

UpdatableComposite::UpdatableComposite(std::vector<sp<Updatable>> updatables)
    : _updatables(std::move(updatables))
{
}

bool UpdatableComposite::update(uint64_t timestamp)
{
    bool dirty = false;
    for(const sp<Updatable>& i : _updatables)
        dirty = i->update(timestamp) || dirty;
    return dirty;
}

}
