#include "renderer/util/animation_type.h"

#include "core/base/expectation.h"
#include "core/util/numeric_type.h"

#include "renderer/inf/animation.h"


namespace ark {

sp<AnimationUploader> AnimationType::makeInput(const sp<Animation>& self, const sp<Numeric>& tick, const sp<Runnable>& onComplete)
{
    sp<Numeric> t = tick;

    if(onComplete)
    {
        float animationDuration = AnimationType::ticks(self);
        DCHECK(t->val() <= animationDuration, "Animation has already completed");
        sp<ExpectationF> exp = NumericType::atMost(t, sp<Numeric::Const>::make(animationDuration));
        exp->observer()->addCallback(onComplete);
        t = std::move(exp);
    }

    return self->makeInput(std::move(t));
}

float AnimationType::ticks(const sp<Animation>& self)
{
    return self->duration();
}

float AnimationType::tps(const sp<Animation>& self)
{
    return self->tps();
}

float AnimationType::duration(const sp<Animation>& self)
{
    return self->duration() / self->tps();
}

const std::vector<String>& AnimationType::nodeNames(const sp<Animation>& self)
{
    return self->nodeNames();
}

}
