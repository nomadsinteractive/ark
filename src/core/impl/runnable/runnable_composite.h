#pragma once

#include "core/forwarding.h"
#include "core/inf/runnable.h"
#include "core/types/shared_ptr.h"

namespace ark {

class RunnableComposite final : public Runnable {
public:
    RunnableComposite(Vector<sp<Runnable>> composites);

    void run() override;

private:
    Vector<sp<Runnable>> _composites;
};

}
