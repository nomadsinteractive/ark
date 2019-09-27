#include "app/impl/tracker/tracker_grid.h"

#include "core/base/bean_factory.h"
#include "core/inf/variable.h"
#include "core/util/math.h"
#include "core/util/log.h"

namespace ark {

namespace {

class TrackedPosition : public Vec {
public:
    TrackedPosition(int32_t id, const sp<TrackerGrid::Stub>& stub, const sp<Vec>& position, const sp<Vec>& size)
        : _id(id), _stub(stub), _position(position), _size(size) {
        const V p = _position->val();
        const V s = _size->val();
        _stub->create(_id, p, s);
    }
    ~TrackedPosition() {
        _stub->remove(_id);
    }

    virtual V val() override {
        const V p = _position->val();
        const V s = _size->val();
        _stub->update(_id, p, s);
        return p;
    }

private:
    int32_t _id;
    sp<TrackerGrid::Stub> _stub;
    sp<Vec> _position;
    sp<Vec> _size;

};

}

TrackerGrid::TrackerGrid(const V& cell)
    : _stub(sp<Stub>::make(cell))
{
}

sp<Vec> TrackerGrid::create(int32_t id, const sp<Vec>& position, const sp<Vec>& size)
{
    return sp<TrackedPosition>::make(id, _stub, position, size);
}

void TrackerGrid::remove(int32_t id)
{
    _stub->remove(id);
}

std::unordered_set<int32_t> TrackerGrid::search(const V& position, const V& size)
{
    return _stub->search(position, size);
}

TrackerGrid::Stub::Stub(const V& cell)
{
    for(int32_t i = 0; i < DIMENSIONS; i++)
    {
        _axes[i]._stride = static_cast<int32_t>(cell[i]);
        DASSERT(_axes[i]._stride > 0);
    }
}

void TrackerGrid::Stub::remove(int32_t id)
{
    for(int32_t i = 0; i < DIMENSIONS; i++)
        _axes[i].remove(id);
}

void TrackerGrid::Stub::create(int32_t id, const V& position, const V& size)
{
    for(int32_t i = 0; i < DIMENSIONS; i++)
    {
        float p = position[i];
        float s = size[i];
        _axes[i].create(id, p, p - s / 2.0f, p + s / 2.0f);
    }
}

void TrackerGrid::Stub::update(int32_t id, const V& position, const V& size)
{
    for(int32_t i = 0; i < DIMENSIONS; i++)
    {
        float p = position[i];
        float s = size[i];
        _axes[i].update(id, p, p - s / 2.0f, p + s / 2.0f);
    }
}

std::unordered_set<int32_t> TrackerGrid::Stub::search(const V& position, const V& size) const
{
    std::unordered_set<int32_t> candidates = _axes[0].search(position[0] - size[0] / 2.0f, position[0] + size[0] / 2.0f);
    for(int32_t i = 1; i < DIMENSIONS && !candidates.empty(); i++)
    {
        const std::unordered_set<int32_t> s1 = std::move(candidates);
        const std::unordered_set<int32_t> s2 = _axes[i].search(position[i] - size[i] / 2.0f, position[i] + size[i] / 2.0f);
        for(int32_t i : s1)
            if(s2.find(i) != s2.end())
                candidates.insert(i);
    }
    return candidates;
}

void TrackerGrid::Axis::create(int32_t id, float position, float low, float high)
{
    int32_t mp = Math::modFloor<int32_t>(static_cast<int32_t>(position), _stride);
    int32_t remainder;
    int32_t begin = Math::divmod(static_cast<int32_t>(low), _stride, remainder);
    int32_t end = Math::divmod(static_cast<int32_t>(high), _stride, remainder) + 1;
    const Range cur(mp, begin, end);
    updateRange(id, cur, Range());
}

void TrackerGrid::Axis::update(int32_t id, float position, float low, float high)
{
    int32_t mp = Math::modFloor<int32_t>(static_cast<int32_t>(position), _stride);
    const auto iter = _trackee_ranges.find(id);
    if(iter != _trackee_ranges.end() && iter->second._position != mp)
    {
        int32_t remainder;
        int32_t begin = Math::divmod(static_cast<int32_t>(low), _stride, remainder);
        int32_t end = Math::divmod(static_cast<int32_t>(high), _stride, remainder) + 1;
        const Range cur(mp, begin, end);
        const Range prev = iter->second;
        updateRange(id, cur, prev);
    }
}

void TrackerGrid::Axis::updateRange(int32_t id, const Range& cur, const Range& prev)
{
    for(int32_t i = prev._begin; i < prev._end; i++)
        if(!cur.within(i))
            remove(id, i);

    for(int32_t i = cur._begin; i < cur._end; i++)
        if(!prev.within(i))
            _trackee_range_ids.insert(std::make_pair(i, id));

    _trackee_ranges[id] = cur;
}

std::unordered_set<int32_t> TrackerGrid::Axis::search(float low, float high) const
{
    std::unordered_set<int32_t> candidates;
    int32_t remainder;
    int32_t begin = Math::divmod(static_cast<int32_t>(low), _stride, remainder);
    int32_t end = Math::divmod(static_cast<int32_t>(high), _stride, remainder) + 1;
    for(int32_t i = begin; i < end; i++)
    {
        const auto range = _trackee_range_ids.equal_range(i);
        for(auto iter = range.first; iter != range.second; ++iter)
            candidates.insert(iter->second);
    }
    return candidates;
}

void TrackerGrid::Axis::remove(int32_t id)
{
    const auto it1 = _trackee_ranges.find(id);
    if(it1 != _trackee_ranges.end())
    {
        const Range& p = it1->second;

        for(int32_t i = p._begin; i < p._end; i++)
            remove(id, i);

        _trackee_ranges.erase(it1);
    }
}

void TrackerGrid::Axis::remove(int32_t id, int32_t rangeId)
{
    const auto range = _trackee_range_ids.equal_range(rangeId);
    for(auto iter = range.first; iter != range.second; ++iter)
        if(id == iter->second)
        {
            iter = _trackee_range_ids.erase(iter);
            if(iter == range.second)
                break;
        }
}

TrackerGrid::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _cell(factory.ensureBuilder<Vec>(manifest, "cell"))
{
}

sp<Tracker> TrackerGrid::BUILDER::build(const Scope& args)
{
    return sp<TrackerGrid>::make(_cell->build(args)->val());
}

TrackerGrid::Axis::Range::Range()
    : _position(0), _begin(0), _end(0)
{
}

TrackerGrid::Axis::Range::Range(int32_t position, int32_t begin, int32_t end)
    : _position(position), _begin(begin), _end(end)
{
}

bool TrackerGrid::Axis::Range::within(int32_t r) const
{
    return r >= _begin && r < _end;
}

}
