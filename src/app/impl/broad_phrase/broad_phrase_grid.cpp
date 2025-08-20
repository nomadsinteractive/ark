#include "app/impl/broad_phrase/broad_phrase_grid.h"

#include "app/inf/broad_phrase.h"
#include "app/inf/broad_phrase_callback.h"
#include "core/base/bean_factory.h"
#include "core/util/math.h"

namespace ark {


class BroadPhraseGrid::Stub final : public BroadPhrase::Coordinator {
public:
    Stub(int32_t dimension, const V3& cell)
        : _dimension(dimension), _axes(new Axis[dimension])
    {
        CHECK(_dimension < 4, "Dimension should be either 2(V2) or 3(V3)");

        for(int32_t i = 0; i < _dimension; i++)
        {
            _axes[i]._stride = static_cast<int32_t>(cell[i]);
            DASSERT(_axes[i]._stride > 0);
        }
    }
    ~Stub() override
    {
        delete[] _axes;
    }

    void remove(RefId id) override
    {
        for(int32_t i = 0; i < _dimension; i++)
            _axes[i].remove(id);
    }

    void create(RefId id, const V3& position, const V3& size) override
    {
        for(int32_t i = 0; i < _dimension; i++)
        {
            float p = position[i];
            float s = size[i];
            _axes[i].create(id, p, p - s / 2.0f, p + s / 2.0f);
        }
    }
    
    void update(RefId id, const V3& position, const V3& size) override
    {
        for(int32_t i = 0; i < _dimension; i++)
        {
            float p = position[i];
            float s = size[i];
            _axes[i].update(id, p, p - s / 2.0f, p + s / 2.0f);
        }
    }

    void search(BroadPhraseCallback& callback, const V3& position, const V3& size) const
    {
        HashSet<RefId> candidates = _axes[0].search(position[0] - size[0] / 2.0f, position[0] + size[0] / 2.0f);
        for(int32_t i = 1; i < _dimension && !candidates.empty(); i++)
        {
            const HashSet<RefId> s1 = std::move(candidates);
            const HashSet<RefId> s2 = _axes[i].search(position[i] - size[i] / 2.0f, position[i] + size[i] / 2.0f);
            for(int32_t i : s1)
                if(s2.contains(i))
                {
                    callback.onRigidbodyCandidate(i);
                    candidates.insert(i);
                }
        }
    }

private:
    int32_t _dimension;
    Axis* _axes;
};
    
BroadPhraseGrid::BroadPhraseGrid(int32_t dimension, const V3& cell)
    : _stub(sp<Stub>::make(dimension, cell))
{
}

sp<BroadPhrase::Coordinator> BroadPhraseGrid::requestCoordinator()
{
    return _stub.cast<Coordinator>();
}

void BroadPhraseGrid::search(BroadPhraseCallback& callback, const V3 position, const V3 size)
{
    _stub->search(callback, position, size);
}

void BroadPhraseGrid::rayCast(BroadPhraseCallback& callback, const V3 from, const V3 to, const sp<CollisionFilter>& /*collisionFilter*/)
{
    search(callback, V3((from + to) / 2, 0), V3(std::abs(from.x() - to.x()), std::abs(from.y() - to.y()), 0));
}

void BroadPhraseGrid::Axis::create(RefId id, float position, float low, float high)
{
    const int32_t mp = Math::modFloor(static_cast<int32_t>(position), _stride);
    int32_t remainder;
    const int32_t begin = Math::divmod(static_cast<int32_t>(low), _stride, remainder);
    const int32_t end = Math::divmod(static_cast<int32_t>(high), _stride, remainder) + 1;
    const Range cur(mp, begin, end);
    updateRange(id, cur, Range());
}

void BroadPhraseGrid::Axis::update(RefId id, float position, float low, float high)
{
    const int32_t mp = Math::modFloor(static_cast<int32_t>(position), _stride);
    if(const auto iter = _trackee_ranges.find(id); iter != _trackee_ranges.end() && iter->second._position != mp)
    {
        int32_t remainder;
        const int32_t begin = Math::divmod(static_cast<int32_t>(low), _stride, remainder);
        const int32_t end = Math::divmod(static_cast<int32_t>(high), _stride, remainder) + 1;
        const Range cur(mp, begin, end);
        const Range prev = iter->second;
        updateRange(id, cur, prev);
    }
}

void BroadPhraseGrid::Axis::updateRange(RefId id, const Range& cur, const Range& prev)
{
    for(int32_t i = prev._begin; i < prev._end; i++)
        if(!cur.within(i))
            remove(id, i);

    for(int32_t i = cur._begin; i < cur._end; i++)
        if(!prev.within(i))
            _trackee_range_ids.insert(std::make_pair(i, id));

    _trackee_ranges[id] = cur;
}

HashSet<RefId> BroadPhraseGrid::Axis::search(float low, float high) const
{
    HashSet<RefId> candidates;
    int32_t remainder;
    const int32_t begin = Math::divmod(static_cast<int32_t>(low), _stride, remainder);
    const int32_t end = Math::divmod(static_cast<int32_t>(high), _stride, remainder) + 1;
    for(int32_t i = begin; i < end; i++)
    {
        const auto [k, v] = _trackee_range_ids.equal_range(i);
        for(auto iter = k; iter != v; ++iter)
            candidates.insert(iter->second);
    }
    return candidates;
}

void BroadPhraseGrid::Axis::remove(const RefId id)
{
    if(const auto iter = _trackee_ranges.find(id); iter != _trackee_ranges.end())
    {
        const Range& p = iter->second;

        for(int32_t i = p._begin; i < p._end; i++)
            remove(id, i);

        _trackee_ranges.erase(iter);
    }
}

void BroadPhraseGrid::Axis::remove(const RefId id, const int32_t rangeId)
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
    : _dimension(Documents::getAttribute<int32_t>(manifest, "dimension", 2)), _cell(factory.ensureBuilder<Vec3>(manifest, "cell"))
{
}

sp<BroadPhrase> BroadPhraseGrid::BUILDER::build(const Scope& args)
{
    return sp<BroadPhrase>::make<BroadPhraseGrid>(_dimension, _cell->build(args)->val());
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
