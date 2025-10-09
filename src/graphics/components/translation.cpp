#include "graphics/components/translation.h"

#include <glm/gtc/matrix_transform.hpp>

#include "core/impl/variable/variable_dirty_mark.h"
#include "core/impl/variable/variable_op1.h"
#include "core/impl/variable/variable_wrapper.h"

#include "graphics/base/mat.h"
#include "graphics/base/v3.h"

namespace ark {

namespace {

class TranslateOP {
public:
    M4 operator()(const V3 v1) const {
        return glm::translate(glm::mat4(1.0f), *reinterpret_cast<const glm::vec3*>(&v1));
    }
};

}

Translation::Translation(sp<Vec3> translate)
    : Translation(sp<Vec3Wrapper>::make(std::move(translate)))
{
}

Translation::Translation(const sp<Vec3Wrapper>& vec3Wrapper)
    : _delegate(vec3Wrapper), _wrapper(vec3Wrapper)
{
}

Translation::Translation(sp<Vec3> delegate, sp<Wrapper<Vec3>> wrapper)
    : _delegate(std::move(delegate)), _wrapper(std::move(wrapper))
{
}

bool Translation::update(const uint64_t timestamp)
{
    return _delegate->update(timestamp);
}

V3 Translation::val()
{
    return _delegate->val();
}

void Translation::reset(sp<Vec3> position)
{
    VariableDirtyMark<V3>::markDirty(_wrapper, std::move(position));
}

sp<Mat4> Translation::toMatrix() const
{
    return sp<Mat4>::make<VariableOP1<M4, V3>>(TranslateOP(), _delegate);
}

}
