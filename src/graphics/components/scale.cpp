#include "graphics/components/scale.h"

#include <glm/gtc/matrix_transform.hpp>

#include "core/impl/variable/variable_dirty_mark.h"
#include "core/impl/variable/variable_op1.h"

#include "graphics/base/mat.h"
#include "graphics/base/v3.h"

namespace ark {

namespace {

class ScaleOP {
public:
    M4 operator()(const V3 v1) const {
        return glm::scale(glm::mat4(1.0f), *reinterpret_cast<const glm::vec3*>(&v1));
    }
};

}

Scale::Scale(sp<Vec3> scale)
    : Wrapper(std::move(scale))
{
}

bool Scale::update(const uint64_t timestamp)
{
    return _wrapped->update(timestamp);
}

V3 Scale::val()
{
    return _wrapped->val();
}

void Scale::reset(sp<Vec3> scale)
{
    VariableDirtyMark<V3>::markDirty(*this, std::move(scale));
}

sp<Mat4> Scale::toMatrix() const
{
    return sp<Mat4>::make<VariableOP1<M4, V3>>(ScaleOP(), _wrapped);
}

}
