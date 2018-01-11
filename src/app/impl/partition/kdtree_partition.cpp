#include "app/impl/partition/kdtree_partition.h"

#include <iostream>
#include "core/collection/list.h"
#include "core/inf/variable.h"

#include "graphics/base/v2.h"

namespace ark {

KDTreePartition::Adapter::Point::Point(uint32_t id, const sp<VV>& position)
    : _id(id), _position(position)
{
    update();
}

void KDTreePartition::Adapter::Point::update()
{
    const V v2 = _position->val();
    _v[0] = v2.x();
    _v[1] = v2.y();
}

KDTreePartition::KDTreePartition()
    : _updated_count(0)
{
    buildIndex();
}

uint32_t KDTreePartition::addPoint(const sp<VV>& pos)
{
    rebuildIfNeeded();
    if(_id_recycler.size() > 0)
    {
        const auto head = _id_recycler.begin();
        uint32_t recycledId = *head;
        _adapter.pts[recycledId] = Adapter::Point(recycledId, pos);
        _tree->addPoints(recycledId, recycledId);
        _id_recycler.erase(head);
        return recycledId;
    }
    uint32_t id = _adapter.pts.size();
    _adapter.pts.push_back(Adapter::Point(id, pos));
    _tree->addPoints(id, id);
    return id;
}

void KDTreePartition::updatePoint(uint32_t id)
{
    getPoint(id).update();
}

void KDTreePartition::removePoint(uint32_t id)
{
    DCHECK(_id_recycler.find(id) == _id_recycler.end(), "Id: %d has been deleted already", id);
    _tree->removePoint(id);
    _id_recycler.insert(id);
    rebuildIfNeeded();
}

List<uint32_t> KDTreePartition::within(float x, float y, float radius)
{
	List<uint32_t> result;
	const float pt[] = {x, y};

    std::vector<std::pair<uint32_t, float>> indices_dists;
    nanoflann::RadiusResultSet<float, uint32_t> resultSet(radius * radius, indices_dists);
    _tree->findNeighbors(resultSet, pt, nanoflann::SearchParams());

    for(const std::pair<uint32_t, float>& i : indices_dists)
        result.push_back(i.first);

	return result;
}

KDTreePartition::Adapter::Point& KDTreePartition::getPoint(uint32_t id)
{
    DCHECK(id < _adapter.pts.size(), "Index out bounds, index: %d", id);
    return _adapter.pts[id];
}

void KDTreePartition::buildIndex()
{
    const std::vector<Adapter::Point> pts = std::move(_adapter.pts);
    _tree.reset(new kd_tree_t(2, _adapter, nanoflann::KDTreeSingleIndexAdaptorParams()));
    if(pts.size() > 0)
    {
        _adapter.pts = std::move(pts);
        _tree->addPoints(0, _adapter.pts.size() - 1);
        for(uint32_t i : _id_recycler)
            _tree->removePoint(i);
    }
}

bool KDTreePartition::rebuildIfNeeded()
{
    if(++_updated_count > _adapter.pts.size() && _adapter.pts.size() > 10)
    {
        buildIndex();
        _updated_count = 0;
        return false;
    }
    return true;
}

KDTreePartition::DICTIONARY::DICTIONARY()
{
}

sp<Partition> KDTreePartition::DICTIONARY::build(const sp<Scope>& /*args*/)
{
    return sp<Partition>::adopt(new KDTreePartition());
}

}
