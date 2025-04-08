#include "graphics/util/transform_type.h"

#include "core/impl/variable/variable_dirty_mark.h"
#include "core/types/shared_ptr.h"

#include "graphics/impl/transform/transform_impl.h"

namespace ark {

sp<Transform> TransformType::create(sp<Mat4> matrix)
{
    return sp<Transform>::make<TransformImpl>(std::move(matrix));
}

sp<Vec3> TransformType::translation(const sp<Transform>& self)
{
    if(const sp<TransformImpl> transform = self.asInstance<TransformImpl>())
        return transform->translation().toVar();
    return nullptr;
}

void TransformType::setTranslation(const sp<Transform>& self, sp<Vec3> translation)
{
    const sp<TransformImpl> transform = self.ensureInstance<TransformImpl>("Transform is not an instance of TransformImpl");
    transform->setTranslation(std::move(translation));
}

sp<Vec4> TransformType::rotation(const sp<Transform>& self)
{
    if(const sp<TransformImpl> transform = self.asInstance<TransformImpl>())
        return transform->rotation().toVar();
    return nullptr;
}

void TransformType::setRotation(const sp<Transform>& self, sp<Vec4> rotation)
{
    const sp<TransformImpl> transform = self.ensureInstance<TransformImpl>("Transform is not an instance of TransformImpl");
    transform->setRotation(std::move(rotation));
}

sp<Vec3> TransformType::scale(const sp<Transform>& self)
{
    if(const sp<TransformImpl> transform = self.asInstance<TransformImpl>())
        return transform->scale().toVar();
    return nullptr;
}

void TransformType::setScale(const sp<Transform>& self, sp<Vec3> scale)
{
    const sp<TransformImpl> transform = self.ensureInstance<TransformImpl>("Transform is not an instance of TransformImpl");
    transform->setScale(std::move(scale));
}

void TransformType::reset(const sp<Transform>& self, sp<Mat4> matrix)
{
    const sp<TransformImpl> transform = self.ensureInstance<TransformImpl>("Transform is not an instance of TransformImpl");
    transform->reset(std::move(matrix));
}

}
