#include "renderer/util/animation_type.h"


#include "core/epi/notifier.h"
#include "core/base/expectation.h"
#include "core/util/numeric_type.h"

#include "renderer/inf/animation.h"


namespace ark {

sp<AnimationInput> AnimationType::makeTransforms(const sp<Animation>& self, const sp<Numeric>& duration, const sp<Runnable>& onComplete)
{
    sp<Numeric> t = duration;

    if(onComplete)
    {
        float animationDuration = self->duration();
        DCHECK(t->val() <= animationDuration, "Animation has already completed");
        sp<Expectation> exp = NumericType::atMost(t, sp<Numeric::Const>::make(animationDuration));
        exp->addObserver(onComplete, true);
        t = std::move(exp);
    }

    return self->makeTransforms(std::move(t));
}

float AnimationType::duration(const sp<Animation>& self)
{
    return self->duration();
}

const std::vector<String>& AnimationType::nodeNames(const sp<Animation>& self)
{
    return self->nodeNames();
}

}
