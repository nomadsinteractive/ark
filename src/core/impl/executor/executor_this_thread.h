#pragma once

#include "core/inf/executor.h"

namespace ark {

class ExecutorThisThread : public Executor {
public:

    void execute(sp<Runnable> task) override;

};

}
