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

    void requestActivate(const bool active)
    {
        _activated = active;
        _timestamp.markDirty();
    }

    void requestSuppress(const bool suppress)
    {
        if(_active)
        {
            _suppressed = suppress;
            _timestamp.markDirty();
        }
    }

    bool _suppressed = false;
    bool _activated = false;
    bool _active = false;

    Set<const State*> _supporting_states;
    Timestamp _timestamp;
};

class State::BooleanStateSuppressed : public Boolean {
public:
    BooleanStateSuppressed(const sp<Stub>& stub)
        : _stub(stub)
    {
    }

    bool update(const uint32_t tick) override
    {
        return _stub->update(tick);
    }

    bool val() override
    {
        return _stub->_suppressed;
    }

private:
    sp<Stub> _stub;
};

State::State(sp<Runnable> onActivate, sp<Runnable> onDeactivate)
    : _on_activate(std::move(onActivate)), _on_deactivate(std::move(onDeactivate)), _stub(sp<Stub>::make())
{
}

sp<Boolean> State::active() const
{
    return _stub;
}

sp<Boolean> State::suppressed() const
{
    return sp<Boolean>::make<BooleanStateSuppressed>(_stub);
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
            else if(i->_link_type == LINK_TYPE_SUPPORT)
                i->_start.propagateSupporting(*this);

        for(const sp<Link>& i : _out_links)
            if(i->_link_type == LINK_TYPE_PROPAGATE)
                i->_end.propagateActive(*this);

        onActivate();
    }
}

void State::deactivate()
{
    _stub->_activated = false;
    _stub->_suppressed = false;
    if(_stub->_supporting_states.empty())
    {
        onDeactivate();

        for(const sp<Link>& i : _in_links)
            if(i->_link_type == LINK_TYPE_TRANSIT && !i->_start.isActive())
                i->_start.propagateUnsuppress(*this);
            else if(i->_link_type == LINK_TYPE_SUPPORT)
                i->_start.propagateUnsupporting(*this);

        for(const sp<Link>& i : _out_links)
            if(i->_link_type == LINK_TYPE_PROPAGATE || i->_link_type == LINK_TYPE_SUPPORT)
                i->_end.propagateDeactive(*this);
    }
}

void State::createLink(const LinkType linkType, State& nextState)
{
    sp<Link> link = sp<Link>::make(Link{linkType, *this, nextState});
    nextState._in_links.emplace_back(link);
    _out_links.push_back(std::move(link));
}

void State::propagateSuppress(const State& from)
{
    if(!_stub->_suppressed && _stub->_active)
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
    if(_stub->_activated)
        activate();
    else if(_stub->_suppressed)
        onActivate();
    _stub->_suppressed = false;

    for(const sp<Link>& i : _out_links)
        if((i->_link_type == LINK_TYPE_SUPPORT || i->_link_type == LINK_TYPE_PROPAGATE) && &i->_end != &from)
            i->_end.propagateUnsuppress(*this);
}

void State::propagateSupporting(const State& from)
{
    _stub->_supporting_states.insert(&from);
    for(const sp<Link>& i : _in_links)
        if(i->_link_type == LINK_TYPE_SUPPORT)
            i->_end.propagateSupporting(*this);
}

void State::propagateUnsupporting(const State& from)
{
    if(const auto iter = _stub->_supporting_states.find(&from); iter != _stub->_supporting_states.end())
        _stub->_supporting_states.erase(iter);

    if(_stub->_active && !_stub->_activated && _stub->_supporting_states.empty())
        deactivate();

    for(const sp<Link>& i : _in_links)
        if(i->_link_type == LINK_TYPE_SUPPORT)
            i->_end.propagateUnsupporting(*this);
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
    {
        onDeactivate();
        _stub->_suppressed = false;
    }
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
