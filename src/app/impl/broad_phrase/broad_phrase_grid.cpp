#include "app/impl/broad_phrase/broad_phrase_grid.h"

#include "core/base/bean_factory.h"
#include "core/util/math.h"

namespace ark {

BroadPhraseGrid::BroadPhraseGrid(uint32_t dimension, const V3& cell)
    : _stub(sp<Stub>::make(dimension, cell))
{
}

sp<Vec3> BroadPhraseGrid::create(int32_t id, const sp<Vec3>& position, const sp<Vec3>& size)
{
    return sp<TrackedPosition>::make(id, _stub, position, size);
}

void BroadPhraseGrid::remove(int32_t id)
{
    _stub->remove(id);
}

BroadPhrase::Result BroadPhraseGrid::search(const V3& position, const V3& size)
{
    return BroadPhrase::Result(_stub->search(position, size), {});
}

BroadPhrase::Result BroadPhraseGrid::rayCast(const V3& from, const V3& to)
{
    return search(V3((from + to) / 2, 0), V3(std::abs(from.x() - to.x()), std::abs(from.y() - to.y()), 0));
}

BroadPhraseGrid::Stub::Stub(uint32_t dimension, const V3& cell)
    : _dimension(dimension), _axes(new Axis[dimension])
{
    DCHECK(_dimension < 4, "Dimension should be either 2(V2) or 3(V3)");

    for(uint32_t i = 0; i < _dimension; i++)
    {
        _axes[i]._stride = static_cast<int32_t>(cell[i]);
        DASSERT(_axes[i]._stride > 0);
    }
}

BroadPhraseGrid::Stub::~Stub()
{
    delete[] _axes;
}

void BroadPhraseGrid::Stub::remove(int32_t id)
{
    for(uint32_t i = 0; i < _dimension; i++)
        _axes[i].remove(id);
}

void BroadPhraseGrid::Stub::create(int32_t id, const V3& position, const V3& size)
{
    for(uint32_t i = 0; i < _dimension; i++)
    {
        float p = position[i];
        float s = size[i];
        _axes[i].create(id, p, p - s / 2.0f, p + s / 2.0f);
    }
}

void BroadPhraseGrid::Stub::update(int32_t id, const V3& position, const V3& size)
{
    for(uint32_t i = 0; i < _dimension; i++)
    {
        float p = position[i];
        float s = size[i];
        _axes[i].update(id, p, p - s / 2.0f, p + s / 2.0f);
    }
}

std::unordered_set<int32_t> BroadPhraseGrid::Stub::search(const V3& position, const V3& size) const
{
    std::unordered_set<int32_t> candidates = _axes[0].search(position[0] - size[0] / 2.0f, position[0] + size[0] / 2.0f);
    for(uint32_t i = 1; i < _dimension && !candidates.empty(); i++)
    {
        const std::unordered_set<int32_t> s1 = std::move(candidates);
        const std::unordered_set<int32_t> s2 = _axes[i].search(position[i] - size[i] / 2.0f, position[i] + size[i] / 2.0f);
        for(int32_t i : s1)
            if(s2.find(i) != s2.end())
                candidates.insert(i);
    }
    return candidates;
}

void BroadPhraseGrid::Axis::create(int32_t id, float position, float low, float high)
{
    int32_t mp = Math::modFloor<int32_t>(static_cast<int32_t>(position), _stride);
    int32_t remainder;
    int32_t begin = Math::divmod(static_cast<int32_t>(low), _stride, remainder);
    int32_t end = Math::divmod(static_cast<int32_t>(high), _stride, remainder) + 1;
    const Range cur(mp, begin, end);
    updateRange(id, cur, Range());
}

void BroadPhraseGrid::Axis::update(int32_t id, float position, float low, float high)
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

void BroadPhraseGrid::Axis::updateRange(int32_t id, const Range& cur, const Range& prev)
{
    for(int32_t i = prev._begin; i < prev._end; i++)
        if(!cur.within(i))
            remove(id, i);

    for(int32_t i = cur._begin; i < cur._end; i++)
        if(!prev.within(i))
            _trackee_range_ids.insert(std::make_pair(i, id));

    _trackee_ranges[id] = cur;
}

std::unordered_set<int32_t> BroadPhraseGrid::Axis::search(float low, float high) const
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

void BroadPhraseGrid::Axis::remove(int32_t id)
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

void BroadPhraseGrid::Axis::remove(int32_t id, int32_t rangeId)
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

BroadPhraseGrid::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _dimension(Documents::getAttribute(manifest, "dimension", 2)), _cell(factory.ensureBuilder<Vec3>(manifest, "cell"))
{
}

sp<BroadPhrase> BroadPhraseGrid::BUILDER::build(const Scope& args)
{
    return sp<BroadPhraseGrid>::make(_dimension, _cell->build(args)->val());
}

BroadPhraseGrid::Axis::Range::Range()
    : _position(0), _begin(0), _end(0)
{
}

BroadPhraseGrid::Axis::Range::Range(int32_t position, int32_t begin, int32_t end)
    : _position(position), _begin(begin), _end(end)
{
}

bool BroadPhraseGrid::Axis::Range::within(int32_t r) const
{
    return r >= _begin && r < _end;
}

}
