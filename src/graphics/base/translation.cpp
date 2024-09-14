#include "graphics/base/translation.h"

#include <glm/gtc/matrix_transform.hpp>

#include "core/impl/variable/variable_op1.h"

#include "graphics/base/mat.h"
#include "graphics/base/v3.h"

namespace ark {

namespace {

class Translate {
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

bool Translation::update(uint64_t timestamp)
{
    return _wrapped->update(timestamp);
}

V3 Translation::val()
{
    return _wrapped->val();
}

sp<Mat4> Translation::toMatrix() const
{
    return sp<VariableOP1<M4, V3>>::make(Translate(), _wrapped);
}
}
