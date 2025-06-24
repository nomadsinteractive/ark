#pragma once

#include "core/inf/executor.h"

namespace ark {

class ExecutorThisThread final : public Executor {
public:

    void execute(const sp<Runnable>& task) override;

};

}
