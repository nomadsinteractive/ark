#pragma once

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/concurrent/lf_stack.h"
#include "core/types/shared_ptr.h"

namespace ark {

class ARK_API MessageLoop {
public:
    MessageLoop(sp<Variable<uint64_t>> clock);
    MessageLoop(sp<Variable<uint64_t>> clock, sp<Executor> executor);

//  [[script::bindings::auto]]
    void post(sp<Runnable> runnable, float delay, sp<Boolean> canceled = nullptr);
    void post(std::function<void()> task, float delay, sp<Boolean> canceled = nullptr);
//  [[script::bindings::auto]]
    void schedule(sp<Runnable> runnable, float interval, sp<Boolean> canceled = nullptr);

    void pollOnce();

private:
    class Task;

    void requestNextTask(sp<Task> task);

private:
    sp<Variable<uint64_t>> _clock;
    sp<Executor> _executor;

    List<sp<Task>> _tasks;
    LFStack<sp<Task>> _scheduled;
};

}
