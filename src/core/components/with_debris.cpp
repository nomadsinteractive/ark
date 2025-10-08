#include "core/components/with_debris.h"

#include "core/types/weak_ptr.h"

namespace ark {

struct WithDebris::Tracker {
    Vector<WeakPtr<Debris>> _debris;

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

void WithDebris::onPoll(WiringContext& context)
{
    if(!context.hasComponent<Tracker>())
        context.setComponent(sp<Tracker>::make());
}

void WithDebris::onWire(const WiringContext& context, const Box& self)
{
    Vector<WeakPtr<Debris>> debris = std::move(_tracker->_debris);
    _tracker = context.ensureComponent<Tracker>();

    for(WeakPtr<Debris>& i : debris)
        _tracker->_debris.push_back(std::move(i));
}

void WithDebris::traverse(const Visitor& visitor)
{
    _tracker->traverse(visitor);
}

void WithDebris::track(const Box& obj) const
{
    if(const sp<Debris> debris = obj.as<Debris>())
        _tracker->track(debris);
}

void WithDebris::track(const sp<Debris>& debris) const
{
    _tracker->track(debris);
}

sp<WithDebris::Tracker> WithDebris::ensureTracker(const WiringContext& context)
{
    if(sp<Tracker> tracker = context.getComponent<Tracker>())
        return tracker;

    sp<Tracker> tracker = sp<Tracker>::make();
    const_cast<WiringContext&>(context).setComponent(tracker);
    return tracker;
}

}
