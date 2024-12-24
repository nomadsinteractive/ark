#pragma once

#include <vector>

#include "core/inf/runnable.h"
#include "core/types/shared_ptr.h"

namespace ark {

class RunnableComposite final : public Runnable {
public:
    RunnableComposite(std::vector<sp<Runnable>> composites);

    void run() override;

private:
    std::vector<sp<Runnable>> _composites;
};

}
