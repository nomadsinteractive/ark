#include "core/base/state.h"

#include "core/base/timestamp.h"
#include "core/inf/runnable.h"
#include "core/inf/variable.h"

namespace ark {

struct State::Link {
    LinkType _link_type;
    State& _start;
    State& _end;
};

class State::Stub : public Boolean {
public:

    bool update(const uint32_t tick) override
    {
        return _timestamp.update(tick);
    }

    bool val() override
    {
        return (_active || _activated) && !_suppressed;
    }

    void requestActive(const bool active)
    {
        _activated = active;
        _timestamp.markDirty();
    }

    void requestSuppress(const bool suppress)
    {
        _suppressed = suppress;
        _timestamp.markDirty();
    }

    bool _suppressed = false;
    bool _activated = false;
    bool _active = false;

    Timestamp _timestamp;
};
    
State::State(sp<Runnable> onActivate, sp<Runnable> onDeactivate)
    : _on_activate(std::move(onActivate)), _on_deactivate(std::move(onDeactivate)), _stub(sp<Stub>::make())
{
}

sp<Boolean> State::active() const
{
    return _stub;
}

void State::setActive(const bool active)
{
    if(active)
        activate();
    else
        deactivate();
}

bool State::isActive() const
{
    return _stub->val();
}

void State::activate()
{
    _stub->_activated = true;

    bool shouldActive = false;
    uint32_t numOfSupportStates = 0;
    for(const sp<Link>& i : _in_links)
        if(i->_link_type == LINK_TYPE_SUPPORT)
        {
            numOfSupportStates ++;
            if(i->_start.isActive())
            {
                shouldActive = true;
                break;
            }
        }

    if(shouldActive || numOfSupportStates == 0)
    {
        for(const sp<Link>& i : _in_links)
            if(i->_link_type == LINK_TYPE_TRANSIT)
                i->_start.propagateSuppress(*this);

        for(const sp<Link>& i : _out_links)
            if(i->_link_type == LINK_TYPE_PROPAGATE)
                i->_end.propagateActive(*this);

        onActivate();
    }
}

void State::deactivate()
{
    CHECK_WARN(_stub->_activated, "State is not active");
    _stub->_activated = false;
    _stub->_active = false;
    onDeactivate();

    for(const sp<Link>& i : _in_links)
        if(i->_link_type == LINK_TYPE_TRANSIT && !i->_start.isActive())
            i->_start.propagateUnsuppress(*this);

    for(const sp<Link>& i : _out_links)
        if(i->_link_type == LINK_TYPE_PROPAGATE)
            i->_end.propagateDeactive(*this);
}

void State::createLink(const LinkType linkType, State& nextState)
{
    sp<Link> link = sp<Link>::make(Link{linkType, *this, nextState});
    nextState._in_links.emplace_back(link);
    _out_links.push_back(std::move(link));
}

void State::propagateSuppress(const State& from)
{
    if(!_stub->_suppressed)
    {
        _stub->_suppressed = true;
        onDeactivate();
    }

    for(const sp<Link>& i : _out_links)
        if((i->_link_type == LINK_TYPE_SUPPORT || i->_link_type == LINK_TYPE_PROPAGATE) && &i->_end != &from)
            i->_end.propagateSuppress(*this);
}

void State::propagateUnsuppress(const State& from)
{
    _stub->_suppressed = false;
    if(_stub->_activated)
        onActivate();

    for(const sp<Link>& i : _out_links)
        if((i->_link_type == LINK_TYPE_SUPPORT || i->_link_type == LINK_TYPE_PROPAGATE) && &i->_end != &from)
            i->_end.propagateUnsuppress(*this);
}

void State::propagateActive(const State& from)
{
    if(!_stub->_suppressed)
        onActivate();

    for(const sp<Link>& i : _in_links)
        if(i->_link_type == LINK_TYPE_PROPAGATE && &from != &i->_start)
            i->_start.propagateSuppress(*this);
}

void State::propagateDeactive(const State& from)
{
    bool active = false;
    for(const sp<Link>& i : _in_links)
        if(i->_link_type == LINK_TYPE_PROPAGATE && &from != &i->_start)
        {
            i->_start.propagateUnsuppress(*this);
            active |= i->_start.isActive();
        }
    if(!active)
        onDeactivate();
}

void State::onActivate() const
{
    if(!_stub->_active)
    {
        if(_on_activate)
            _on_activate->run();
        _stub->_active = true;
    }
}

void State::onDeactivate() const
{
    if(_stub->_active)
    {
        if(_on_deactivate)
            _on_deactivate->run();
        _stub->_active = false;
    }
}

}
