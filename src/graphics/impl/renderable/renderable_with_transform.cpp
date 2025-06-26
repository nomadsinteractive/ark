#include "graphics/impl/renderable/renderable_with_transform.h"

#include "graphics/base/render_request.h"

namespace ark {

namespace {

class TransformMatrix final : public Transform {
public:
    TransformMatrix(sp<Transform> transform, const M4& matrix, const V3& origin)
        : _transform(std::move(transform)), _matrix(matrix), _origin(origin) {
    }

    bool update(const uint64_t timestamp) override
    {
        return _transform->update(timestamp);
    }

    Snapshot snapshot() override
    {
        return _transform->snapshot();
    }

    V4 transform(const Snapshot& snapshot, const V4& xyzw) override
    {
        return {MatrixUtil::transform(_matrix, _transform->transform(snapshot, xyzw),_origin), 1.0f};
    }

    M4 toMatrix(const Snapshot& snapshot) override
    {
        return _matrix * _transform->toMatrix(snapshot);
    }

private:
    sp<Transform> _transform;
    M4 _matrix;
    V3 _origin;
};

}

RenderableWithTransform::RenderableWithTransform(sp<Renderable> delegate, sp<Mat4> transform)
    : Wrapper(std::move(delegate)), _transform(std::move(transform))
{
}

Renderable::State RenderableWithTransform::updateState(const RenderRequest& renderRequest)
{
    const bool dirty = _transform->update(renderRequest.timestamp());
    const State state = _wrapped->updateState(renderRequest);
    return dirty ? state | RENDERABLE_STATE_DIRTY : state;
}

Renderable::Snapshot RenderableWithTransform::snapshot(const LayerContextSnapshot& snapshotContext, const RenderRequest& renderRequest, const State state)
{
    Snapshot snapshot = _wrapped->snapshot(snapshotContext, renderRequest, state);
    snapshot._transform = sp<Transform>::make<TransformMatrix>(std::move(snapshot._transform), _transform->val(), -snapshot._position);
    return snapshot;
}

}
