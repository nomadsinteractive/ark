#pragma once

#include <functional>

#include "core/inf/runnable.h"

namespace ark {

class RunnableByFunction final : public Runnable {
public:
    RunnableByFunction(std::function<void()> function);

    void run() override;

private:
    std::function<void()> _function;
};

}
