#include "core/traits/with_debris.h"

#include <vector>

#include "core/types/weak_ptr.h"

namespace ark {

struct WithDebris::Tracker {
    std::vector<WeakPtr<Debris>> _debris;

    void track(const sp<Debris>& debris)
    {
        _debris.emplace_back(debris);
    }

    void traverse(const Visitor& visitor)
    {
        for(auto iter = _debris.begin(); iter != _debris.end(); )
            if(const sp<Debris> debris = iter->lock())
            {
                debris->traverse(visitor);
                ++iter;
            }
            else
                iter = _debris.erase(iter);
    }
};

WithDebris::WithDebris()
    : _tracker(sp<Tracker>::make()) {
}

void WithDebris::track(const sp<Debris>& debris) const
{
    _tracker->track(debris);
}

TypeId WithDebris::onPoll(WiringContext& context)
{
    return constants::TYPE_ID_NONE;
}

void WithDebris::onWire(const WiringContext& context)
{
    std::vector<WeakPtr<Debris>> debris = std::move(_tracker->_debris);
    _tracker = ensureTracker(context);

    for(WeakPtr<Debris>& i : debris)
        _tracker->_debris.push_back(std::move(i));
}

void WithDebris::traverse(const Visitor& visitor)
{
    _tracker->traverse(visitor);
}

sp<WithDebris::Tracker> WithDebris::ensureTracker(const WiringContext& context)
{
    if(sp<Tracker> tracker = context.getComponent<Tracker>())
        return tracker;

    sp<Tracker> tracker = sp<Tracker>::make();
    const_cast<WiringContext&>(context).addComponent(tracker);
    return tracker;
}

}
