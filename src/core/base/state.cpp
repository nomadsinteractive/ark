#include "core/base/state.h"

#include "core/base/state_action.h"
#include "core/base/state_action_strand.h"
#include "core/base/state_link.h"
#include "core/inf/runnable.h"

namespace ark {

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
    for(const sp<StateLink>& i : _in_links)
        if(i->linkType() == LINK_TYPE_SUPPORT)
        {
            numOfSupportStates ++;
            if(i->start().active())
            {
                doActivate();
                break;
            }
        }
    if(numOfSupportStates == 0)
        doActivate();

    if(_active)
    {
        for(const sp<StateLink>& i : _in_links)
            if(i->linkType() == LINK_TYPE_TRANSIT)
                i->start().suppress();

        for(const sp<StateLink>& i : _out_links)
            if(i->linkType() == LINK_TYPE_PROPAGATE && !i->end().active())
                i->start().doActivate();
    }
}

void State::deactivate()
{
    CHECK_WARN(_active, "State is not active");
    doDeactivate();

    for(const sp<StateLink>& i : _in_links)
        if(i->linkType() == LINK_TYPE_TRANSIT && !i->start().active())
            i->start().unsuppress();

    for(const sp<StateLink>& i : _out_links)
        if(i->linkType() == LINK_TYPE_PROPAGATE)
            i->end().backPropagate(*this);
}

void State::createLink(const LinkType linkType, State& nextState)
{
    sp<StateLink> link = sp<StateLink>::make(linkType, *this, nextState);
    nextState._in_links.emplace_back(link);
    _out_links.push_back(std::move(link));
}

void State::suppress()
{
    if(_active)
    {
        _suppressed = true;
        doDeactivate();
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

void State::backPropagate(const State& deactivated)
{
    bool active = false;
    for(const sp<StateLink>& i : _in_links)
        if(i->linkType() == LINK_TYPE_PROPAGATE && &deactivated != &i->start())
        {
            i->start().unsuppress();
            active |= i->start().active();
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
