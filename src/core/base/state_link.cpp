#include "core/base/state_link.h"

#include "core/base/state_machine.h"

namespace ark {

StateLink::StateLink(const State::LinkType linkType, State& start, State& end)
    : _link_type(linkType), _start(start), _end(end)
{
}

State::LinkType StateLink::linkType() const
{
    return _link_type;
}

State& StateLink::start() const
{
    return _start;
}

State& StateLink::end() const
{
    return _end;
}

}
