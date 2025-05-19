#pragma once

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/base/state.h"

namespace ark {

class ARK_API StateLink {
public:
    StateLink(State::LinkType linkType, State& start, State& end);

    State::LinkType linkType() const;
    State& start() const;
    State& end() const;

private:
    State::LinkType _link_type;
    State& _start;
    State& _end;
};

}
