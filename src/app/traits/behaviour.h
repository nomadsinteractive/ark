#pragma once

#include "core/inf/holder.h"
#include "core/types/shared_ptr.h"

namespace ark {

class Behaviour final : public Holder {
public:
    Behaviour(sp<Interpreter> interpreter, Box delegate);

    void traverse(const Visitor& visitor) override;

private:
    sp<Interpreter> _interpreter;
    Box _delegate;
};

}
