#include "app/impl/broad_phrase/broad_phrase_trie.h"

#include "app/inf/broad_phrase.h"
#include "app/inf/broad_phrase_callback.h"
#include "core/base/bean_factory.h"
#include "core/util/math.h"
#include "core/util/log.h"

namespace ark {

class BroadPhraseTrie::Stub final : public BroadPhrase::Coordinator {
public:
    Stub(const int32_t dimension)
        : _dimension(dimension), _axes(new Axis[dimension])
    {
        CHECK(_dimension < 4, "Dimension should be either 2(V2) or 3(V3)");
    }
    ~Stub() override
    {
        delete[] _axes;
    };

    void remove(RefId id) override
    {
        for(int32_t i = 0; i < _dimension; i++)
            _axes[i].remove(id);
    }

    void create(RefId id, const V3& position, const V3& size) override
    {
        for(int32_t i = 0; i < _dimension; i++)
        {
            const float p = position[i];
            const float s = size[i];
            _axes[i].create(id, p - s / 2.0f, p + s / 2.0f);
        }
    }

    void update(RefId id, const V3& position, const V3& size) override
    {
        for(int32_t i = 0; i < _dimension; i++)
        {
            const float p = position[i];
            const float s = size[i];
            _axes[i].update(id, p - s / 2.0f, p + s / 2.0f);
        }
    }

    void search(BroadPhraseCallback& callback, const V3& position, const V3& size) const
    {
        HashSet<RefId> candidates = _axes[0].search(position[0] - size[0] / 2.0f, position[0] + size[0] / 2.0f);
        for(int32_t i = 1; i < _dimension && !candidates.empty(); i++)
        {
            const HashSet<RefId> s1 = std::move(candidates);
            const HashSet<RefId> s2 = _axes[i].search(position[i] - size[i] / 2.0f, position[i] + size[i] / 2.0f);
            for(const RefId j : s1)
                if(s2.contains(j))
                {
                    callback.onRigidbodyCandidate(j);
                    candidates.insert(j);
                }
        }
    }

private:
    int32_t _dimension;
    Axis* _axes;
};

BroadPhraseTrie::BroadPhraseTrie(int32_t dimension)
    : _stub(sp<Stub>::make(dimension))
{
}

sp<BroadPhrase::Coordinator> BroadPhraseTrie::requestCoordinator()
{
    return _stub.cast<Coordinator>();
}

void BroadPhraseTrie::search(BroadPhraseCallback& callback, const V3 position, const V3 size)
{
    _stub->search(callback, position, size);
}

void BroadPhraseTrie::rayCast(BroadPhraseCallback& callback, const V3 from, const V3 to, const sp<CollisionFilter>& /*collisionFilter*/)
{
    search(callback, V3((from + to) / 2, 0), V3(std::abs(from.x() - to.x()), std::abs(from.y() - to.y()), 0));
}

BroadPhraseTrie::BUILDER::BUILDER(BeanFactory& /*factory*/, const document& manifest)
    : _dimension(Documents::getAttribute<int32_t>(manifest, "dimension", 2))
{
}

sp<BroadPhrase> BroadPhraseTrie::BUILDER::build(const Scope& args)
{
    return sp<BroadPhrase>::make<BroadPhraseTrie>(_dimension);
}

BroadPhraseTrie::Axis::Range::Range()
    : _lower(nullptr), _upper(nullptr) {
}

BroadPhraseTrie::Axis::Range::Range(Boundary* lower, Boundary* upper)
    : _lower(lower), _upper(upper) {
}

void BroadPhraseTrie::Axis::create(RefId id, float low, float high)
{
    _ranges[id] = Range(boundaryCreate(_lower_bounds, id, static_cast<int32_t>(std::floor(low))), boundaryCreate(_upper_bounds, id, static_cast<int32_t>(std::ceil(high))));
}

void BroadPhraseTrie::Axis::update(RefId id, float low, float high)
{
    Range& range = ensureRange(id);
    int32_t keyLower = static_cast<int32_t>(std::floor(low));
    int32_t keyUpper = static_cast<int32_t>(std::ceil(high));
    range._lower = boundaryUpdate(_lower_bounds, range._lower, keyLower, id);
    range._upper = boundaryUpdate(_upper_bounds, range._upper, keyUpper, id);
}

void BroadPhraseTrie::Axis::remove(RefId id)
{
    Range& range = ensureRange(id);
    boundaryRemove(_lower_bounds, range._lower, id);
    boundaryRemove(_upper_bounds, range._upper, id);
    _ranges.erase(_ranges.find(id));
}

HashSet<RefId> BroadPhraseTrie::Axis::search(float low, float high) const
{
    std::set<RefId> c1;
    HashSet<RefId> c3;
    const auto keyLower = static_cast<int32_t>(std::floor(low));
    const auto keyUpper = static_cast<int32_t>(std::ceil(high));

    for(auto iter = _lower_bounds.begin(); iter != _lower_bounds.upper_bound(keyUpper); ++iter)
        c1.insert(iter->second.items.begin(), iter->second.items.end());

    for(auto iter = _upper_bounds.lower_bound(keyLower); iter != _upper_bounds.end(); ++iter)
        for(const RefId i : iter->second.items)
            if(c1.contains(i))
                c3.insert(c3.begin(), i);

    return c3;
}

BroadPhraseTrie::Axis::Boundary* BroadPhraseTrie::Axis::boundaryCreate(Map<int32_t, Boundary>& boundaries, RefId id, int32_t key)
{
    Boundary& boundary = boundaries[key];
    if(boundary.items.empty())
        boundary.key = key;
    boundary.items.insert(id);
    return &boundary;
}

BroadPhraseTrie::Axis::Boundary* BroadPhraseTrie::Axis::boundaryUpdate(Map<int32_t, Boundary>& boundaries, Boundary* boundary, int32_t key, RefId id)
{
    if(key != boundary->key)
    {
        boundaryRemove(boundaries, boundary, id);
        return boundaryCreate(boundaries, id, key);
    }
    return boundary;
}

void BroadPhraseTrie::Axis::boundaryRemove(Map<int32_t, Boundary>& boundaries, Boundary* boundary, RefId id)
{
    boundary->items.erase(boundary->items.find(id));
    if(boundary->items.empty())
        boundaries.erase(boundaries.find(boundary->key));
}

BroadPhraseTrie::Axis::Range& BroadPhraseTrie::Axis::ensureRange(int32_t id)
{
    const auto iter = _ranges.find(id);
    DCHECK(iter != _ranges.end(), "Id(%d) not in axis", id);
    return iter->second;
}

}
