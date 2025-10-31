#pragma once

#include "core/concurrent/lf_queue.h"
#include "core/inf/executor.h"
#include "core/inf/runnable.h"
#include "core/types/shared_ptr.h"

namespace ark {

class ExecutorScheduled final : public Executor, public Runnable {
public:

    void execute(const sp<Runnable>& task) override;

    void run() override;

private:
    LFQueue<sp<Runnable>> _queue;
};

}
