#include "graphics/impl/renderable/renderable_with_transform.h"

#include "graphics/base/render_request.h"

namespace ark {

namespace {

class TransformMatrix final : public Transform::Delegate {
public:
    TransformMatrix(sp<Delegate> transform, const M4& matrix, const V3& origin)
        : _transform(std::move(transform)), _matrix(matrix), _origin(origin) {
    }

    bool update(const Transform::Stub& transform, uint64_t timestamp) override
    {
        return _transform->update(transform, timestamp);
    }

    void snapshot(const Transform::Stub& transform, Transform::Snapshot& snapshot) const override
    {
        return _transform->snapshot(transform, snapshot);
    }

    V3 transform(const Transform::Snapshot& snapshot, const V3& position) const override
    {
        return MatrixUtil::transform(_matrix, _transform->transform(snapshot, position),_origin);
    }

    M4 toMatrix(const Transform::Snapshot& snapshot) const override
    {
        return _matrix * _transform->toMatrix(snapshot);
    }

private:
    sp<Delegate> _transform;
    M4 _matrix;
    V3 _origin;
};

}

RenderableWithTransform::RenderableWithTransform(sp<Renderable> delegate, sp<Mat4> transform)
    : Wrapper(std::move(delegate)), _transform(std::move(transform))
{
}

Renderable::StateBits RenderableWithTransform::updateState(const RenderRequest& renderRequest)
{
    const bool dirty = _transform->update(renderRequest.timestamp());
    const StateBits stateBits = _wrapped->updateState(renderRequest);
    return dirty ? static_cast<StateBits>(stateBits | RENDERABLE_STATE_DIRTY) : stateBits;
}

Renderable::Snapshot RenderableWithTransform::snapshot(const LayerContextSnapshot& snapshotContext, const RenderRequest& renderRequest, StateBits state)
{
    Snapshot snapshot = _wrapped->snapshot(snapshotContext, renderRequest, state);
    snapshot._transform._delegate = sp<Transform::Delegate>::make<TransformMatrix>(std::move(snapshot._transform._delegate), _transform->val(), -snapshot._position);
    return snapshot;
}

}
