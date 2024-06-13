#pragma once

#include "core/inf/holder.h"
#include "core/types/shared_ptr.h"

namespace ark {

class Behaviour final : public Holder {
public:
    Behaviour(Box delegate);

    void traverse(const Visitor& visitor) override;

private:
    Box _delegate;
};

}
