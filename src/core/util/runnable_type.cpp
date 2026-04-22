#include "core/util/runnable_type.h"

#include "core/ark.h"
#include "core/inf/runnable.h"
#include "core/types/shared_ptr.h"

#include "app/base/application_context.h"
#include "app/base/application_facade.h"

namespace ark {

namespace {

class RunnableDefered final : public Runnable {
public:
    RunnableDefered(sp<Runnable> runnable, const float delay, sp<Boolean> canceled)
        : _runnable(std::move(runnable)), _delay(delay), _canceled(std::move(canceled)) {
    }

    void run() override
    {
        Ark::instance().applicationContext()->applicationFacade()->post(_runnable, _delay, _canceled);
    }

private:
    sp<Runnable> _runnable;
    float _delay;
    sp<Boolean> _canceled;
};

}

void RunnableType::run(const sp<Runnable>& self)
{
    self->run();
}

sp<Runnable> RunnableType::defer(sp<Runnable> self, float delay, sp<Boolean> canceled)
{
    return sp<Runnable>::make<RunnableDefered>(std::move(self), delay, std::move(canceled));
}

}
