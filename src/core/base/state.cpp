#include "core/base/state.h"

#include "core/inf/runnable.h"

namespace ark {

struct State::Link {
    LinkType _link_type;
    State& _start;
    State& _end;
};
    
State::State(sp<Runnable> onActivate, sp<Runnable> onDeactivate)
    : _on_activate(std::move(onActivate)), _on_deactivate(std::move(onDeactivate)), _active(false), _suppressed(false)
{
}

bool State::active() const
{
    return _active && !_suppressed;
}

void State::activate()
{
    uint32_t numOfSupportStates = 0;
    for(const sp<Link>& i : _in_links)
        if(i->_link_type == LINK_TYPE_SUPPORT)
        {
            numOfSupportStates ++;
            if(i->_start.active())
            {
                doActivate();
                break;
            }
        }
    if(numOfSupportStates == 0)
        doActivate();

    if(_active)
    {
        for(const sp<Link>& i : _in_links)
            if(i->_link_type == LINK_TYPE_TRANSIT)
                i->_start.suppress();

        for(const sp<Link>& i : _out_links)
            if(i->_link_type == LINK_TYPE_PROPAGATE)
                i->_end.propagate(*this);
    }
}

void State::deactivate()
{
    CHECK_WARN(_active, "State is not active");
    doDeactivate();

    for(const sp<Link>& i : _in_links)
        if(i->_link_type == LINK_TYPE_TRANSIT && !i->_start.active())
            i->_start.unsuppress();

    for(const sp<Link>& i : _out_links)
        if(i->_link_type == LINK_TYPE_PROPAGATE)
            i->_end.backPropagate(*this);
}

void State::createLink(const LinkType linkType, State& nextState)
{
    sp<Link> link = sp<Link>::make(Link{linkType, *this, nextState});
    nextState._in_links.emplace_back(link);
    _out_links.push_back(std::move(link));
}

void State::suppress()
{
    if(_active)
    {
        doDeactivate();
        _suppressed = true;
        _active = true;
    }
}

void State::unsuppress()
{
    if(_suppressed)
    {
        _suppressed = false;
        if(_active)
            doActivate();
    }
}

void State::propagate(const State& activated)
{
    if(!_active)
        doActivate();

    for(const sp<Link>& i : _in_links)
        if(i->_link_type == LINK_TYPE_PROPAGATE && &activated != &i->_start)
            i->_start.suppress();
}

void State::backPropagate(const State& deactivated)
{
    bool active = false;
    for(const sp<Link>& i : _in_links)
        if(i->_link_type == LINK_TYPE_PROPAGATE && &deactivated != &i->_start)
        {
            i->_start.unsuppress();
            active |= i->_start.active();
        }
    if(_active && !active)
        doDeactivate();
}

void State::doActivate()
{
    if(_on_activate)
        _on_activate->run();
    _active = true;
}

void State::doDeactivate()
{
    if(_on_deactivate && _active)
        _on_deactivate->run();
    _active = false;
}

}
