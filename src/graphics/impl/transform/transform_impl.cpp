#include "graphics/impl/transform/transform_impl.h"

#include "core/ark.h"
#include "core/base/bean_factory.h"
#include "core/impl/variable/variable_dirty_mark.h"
#include "core/inf/variable.h"
#include "core/types/global.h"
#include "core/util/updatable_util.h"

#include "graphics/base/v3.h"
#include "graphics/impl/transform/transform_trs_2d.h"
#include "graphics/impl/transform/transform_trs_3d.h"
#include "graphics/impl/transform/transform_ts_2d.h"
#include "graphics/impl/transform/transform_ts_3d.h"

namespace ark {

namespace {

class TransformNone final : public Transform {
public:
    TransformNone() = default;

    bool update(uint32_t tick) override
    {
        return false;
    }

    Snapshot snapshot() override
    {
        return {};
    }

    V4 transform(const Snapshot& snapshot, const V4& xyzw) override
    {
        return xyzw;
    }

    M4 toMatrix(const Snapshot& /*snapshot*/) override
    {
        return {};
    }
};

}

class TransformImpl::TransformDelegateMat4 final : public Transform {
public:
    TransformDelegateMat4(const Transform& transform, sp<Mat4> matrix)
        : Transform(transform), _matrix(std::move(matrix)) {
    }

    bool update(uint32_t tick) override
    {
        return UpdatableUtil::update(tick, _stub, _matrix);
    }

    Snapshot snapshot() override
    {
        return {_matrix->val()};
    }

    V4 transform(const Snapshot& snapshot, const V4& xyzw) override
    {
        return MatrixUtil::mul(snapshot.data<M4>(), xyzw);
    }

    M4 toMatrix(const Snapshot& snapshot) override
    {
        return snapshot.data<M4>();
    }

private:
    sp<Mat4> _matrix;
};

TransformImpl::TransformImpl(const TransformType::Type type, sp<Vec4> rotation, sp<Vec3> scale, sp<Vec3> translation)
    : Transform(std::move(rotation), std::move(scale), std::move(translation)), _type(type)
{
    doUpdateDelegate();
}

TransformImpl::TransformImpl(sp<Transform> delegate)
    : Wrapper(std::move(delegate)), _type(TransformType::TYPE_DELEGATED)
{
    doUpdateDelegate();
}

TransformImpl::TransformImpl(sp<Mat4> delegate)
    : Transform(), Wrapper(sp<Transform>::make<TransformDelegateMat4>(*this, std::move(delegate))), _type(TransformType::TYPE_DELEGATED)
{
}

Transform::Snapshot TransformImpl::snapshot()
{
    return _wrapped->snapshot();
}

V4 TransformImpl::transform(const Snapshot& snapshot, const V4& xyzw)
{
    return _wrapped->transform(snapshot, xyzw);
}

M4 TransformImpl::toMatrix(const Snapshot& snapshot)
{
    return _wrapped->toMatrix(snapshot);
}

bool TransformImpl::update(uint32_t tick)
{
    return _wrapped->update(tick);
}

M4 TransformImpl::val()
{
    return _wrapped->val();
}

void TransformImpl::setRotation(sp<Vec4> rotation)
{
    _stub->_rotation.reset(std::move(rotation));
    doUpdateDelegate();
}

void TransformImpl::setScale(sp<Vec3> scale)
{
    _stub->_scale.reset(std::move(scale));
    doUpdateDelegate();
}

void TransformImpl::setTranslation(sp<Vec3> translation)
{
    _stub->_translation.reset(std::move(translation));
    doUpdateDelegate();
}

void TransformImpl::reset(sp<Mat4> transform)
{
    _type = TransformType::TYPE_DELEGATED;
    _wrapped = sp<Transform>::make<TransformDelegateMat4>(*this, std::move(transform));
    doUpdateDelegate();
}

void TransformImpl::doUpdateDelegate()
{
    _stub->_timestamp.markDirty();
    if(_type != TransformType::TYPE_NONE && _type != TransformType::TYPE_DELEGATED)
        _wrapped = makeDelegate();
}

sp<Transform> TransformImpl::makeDelegate() const
{
    CHECK(_type != TransformType::TYPE_DELEGATED, "Delegated Transform may not be updated");

    if(_type == TransformType::TYPE_NONE || (!_stub->_rotation && !_stub->_scale && !_stub->_translation))
        return Global<TransformNone>().cast<Transform>();

    return _stub->_rotation ? makeTransformTRS() : makeTransformTS();
}

sp<Transform> TransformImpl::makeTransformTRS() const
{
    return _type == TransformType::TYPE_LINEAR_2D ? sp<Transform>::make<TransformTRS2D>(*this) : sp<Transform>::make<TransformTRS3D>(*this);
}

sp<Transform> TransformImpl::makeTransformTS() const
{
    return _type == TransformType::TYPE_LINEAR_2D ? sp<Transform>::make<TransformTS2D>(*this) : sp<Transform>::make<TransformTS3D>(*this);
}

}
