#include "graphics/components/scale.h"

#include <glm/gtc/matrix_transform.hpp>

#include "core/impl/variable/variable_dirty_mark.h"
#include "core/impl/variable/variable_op1.h"
#include "core/impl/variable/variable_wrapper.h"

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
    : Scale(sp<Vec3Wrapper>::make(std::move(scale)))
{
}

Scale::Scale(const sp<Vec3Wrapper>& vec3Wrapper)
    : _delegate(vec3Wrapper), _wrapper(vec3Wrapper)
{
}

Scale::Scale(sp<Vec3> delegate, sp<Wrapper<Vec3>> wrapper)
    : _delegate(std::move(delegate)), _wrapper(std::move(wrapper))
{
}

bool Scale::update(const uint64_t timestamp)
{
    return _delegate->update(timestamp);
}

V3 Scale::val()
{
    return _delegate->val();
}

void Scale::reset(sp<Vec3> scale)
{
    VariableDirtyMark<V3>::markDirty(_wrapper, std::move(scale));
}

sp<Mat4> Scale::toMatrix() const
{
    return sp<Mat4>::make<VariableOP1<M4, V3>>(ScaleOP(), _delegate);
}

}
