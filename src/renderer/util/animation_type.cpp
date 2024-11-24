#include "renderer/util/animation_type.h"

#include "core/base/expectation.h"
#include "core/util/numeric_type.h"

#include "renderer/inf/animation.h"


namespace ark {

uint32_t AnimationType::ticks(const sp<Animation>& self)
{
    return static_cast<uint32_t>(self->duration() * self->tps());
}

float AnimationType::tps(const sp<Animation>& self)
{
    return self->tps();
}

float AnimationType::duration(const sp<Animation>& self)
{
    return self->duration();
}

std::vector<std::pair<String, sp<Mat4>>> AnimationType::getNodeTransforms(const sp<Animation>& self, sp<Numeric> time)
{
    return self->getNodeTranforms(std::move(time));
}

}
