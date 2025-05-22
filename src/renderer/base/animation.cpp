#include "renderer/base/animation.h"

#include "node.h"
#include "core/util/integer_type.h"
#include "core/util/math.h"

#include "graphics/base/mat.h"

namespace ark {

namespace {

struct AnimationSession {

    AnimationSession(sp<Integer> tick, const uint32_t durationInTicks, sp<Vector<AnimationFrame>> animationFrames)
        : _tick(std::move(tick)), _duration_in_ticks(durationInTicks), _animation_frames(std::move(animationFrames))
    {
        ASSERT(_duration_in_ticks);
    }

    bool update(const uint64_t timestamp) const
    {
        return _tick->update(timestamp);
    }

    const AnimationFrame& currentFrame() const
    {
        const int32_t frame = Math::floorMod<int32_t>(_tick->val(), _duration_in_ticks);
        return _animation_frames->at(frame);
    }

private:
    sp<Integer> _tick;
    uint32_t _duration_in_ticks;
    sp<Vector<AnimationFrame>> _animation_frames;
};

class LocalMatrix final : public Mat4 {
public:
    LocalMatrix(sp<AnimationSession> session, const uint32_t nodeIndex)
        : _session(std::move(session)), _node_index(nodeIndex) {
    }

    bool update(const uint64_t timestamp) override
    {
        return _session->update(timestamp);
    }

    M4 val() override
    {
        return _session->currentFrame().at(_node_index);
    }

private:
    sp<AnimationSession> _session;
    uint32_t _node_index;
};

class GlobalMatrix final : public Mat4 {
public:
    GlobalMatrix(sp<AnimationSession> session, Vector<std::pair<int32_t, M4>> transformPath)
        : _session(std::move(session)), _transform_path(std::move(transformPath)) {
    }

    bool update(const uint64_t timestamp) override
    {
        return _session->update(timestamp);
    }

    M4 val() override
    {
        M4 transform;
        for(const auto& [k, v] : _transform_path)
            transform = (k >= 0 ? v * _session->currentFrame().at(k) : v) * transform;
        return transform;
    }

private:
    sp<AnimationSession> _session;
    Vector<std::pair<int32_t, M4>> _transform_path;
};

}

Animation::Animation(String name, uint32_t durationInTicks, Table<String, uint32_t> nodes, Vector<AnimationFrame> animationFrames)
    : _name(std::move(name)), _tps(24.0f), _duration(static_cast<float>(durationInTicks) / _tps), _duration_in_ticks(durationInTicks), _nodes(sp<Table<String, uint32_t>>::make(std::move(nodes))), _animation_frames(sp<Vector<AnimationFrame>>::make(std::move(animationFrames)))
{
}

const String& Animation::name() const
{
    return _name;
}

float Animation::duration() const
{
    return _duration;
}

float Animation::tps() const
{
    return _tps;
}

uint32_t Animation::ticks() const
{
    return _duration_in_ticks;
}

Vector<std::pair<String, sp<Mat4>>> Animation::getLocalTransforms(sp<Integer> tick) const
{
    Vector<std::pair<String, sp<Mat4>>> nodeTransforms;

    sp<AnimationSession> session = sp<AnimationSession>::make(std::move(tick), _duration_in_ticks, _animation_frames);
    for(const auto& [name, nodeIdx] : *_nodes)
        nodeTransforms.emplace_back(name, sp<LocalMatrix>::make(session, nodeIdx));

    return nodeTransforms;
}

Vector<std::pair<String, sp<Mat4>>> Animation::getLocalTransforms(sp<Numeric> tick) const
{
    return getLocalTransforms(IntegerType::create(std::move(tick)));
}

sp<Mat4> Animation::getGlobalTransform(const Node& node, sp<Integer> tick) const
{
    Vector<std::pair<int32_t, M4>> transformPath;
    const Node* pNode = &node;
    ASSERT(pNode);
    do
    {
        if(const auto iter = _nodes->find(pNode->name()); iter == _nodes->end())
        {
            if(transformPath.empty() || transformPath.back().first != -1)
                transformPath.emplace_back(-1, pNode->localMatrix());
            else
                transformPath.back().second = pNode->localMatrix() * transformPath.back().second;
        }
        else
            transformPath.emplace_back(static_cast<int32_t>(iter->second), pNode->localMatrix());
        pNode = pNode->parentNode().get();
    } while(pNode);

    sp<AnimationSession> session = sp<AnimationSession>::make(std::move(tick), _duration_in_ticks, _animation_frames);
    return sp<Mat4>::make<GlobalMatrix>(std::move(session), std::move(transformPath));
}

}
