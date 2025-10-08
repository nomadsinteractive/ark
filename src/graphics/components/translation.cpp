#include "graphics/components/translation.h"

#include <glm/gtc/matrix_transform.hpp>

#include "core/impl/variable/variable_dirty_mark.h"
#include "core/impl/variable/variable_op1.h"

#include "graphics/base/mat.h"
#include "graphics/base/v3.h"

namespace ark {

namespace {

class TranslateOP {
public:
    M4 operator()(const V3& v1) const {
        return glm::translate(glm::mat4(1.0f), *reinterpret_cast<const glm::vec3*>(&v1));
    }
};

}

Translation::Translation(sp<Vec3> translate)
    : Wrapper(std::move(translate))
{
}

bool Translation::update(const uint64_t timestamp)
{
    return _wrapped->update(timestamp);
}

V3 Translation::val()
{
    return _wrapped->val();
}

void Translation::reset(sp<Vec3> position)
{
    VariableDirtyMark<V3>::markDirty(*this, std::move(position));
}

sp<Mat4> Translation::toMatrix() const
{
    return sp<Mat4>::make<VariableOP1<M4, V3>>(TranslateOP(), _wrapped);
}

}
