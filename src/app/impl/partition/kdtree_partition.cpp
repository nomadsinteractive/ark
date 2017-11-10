#include "app/impl/partition/kdtree_partition.h"

#include "core/collection/list.h"
#include "core/inf/variable.h"

#include "graphics/base/v2.h"

namespace ark {

KDTreePartition::Adapter::Point::Point(uint32_t id, const sp<VV>& position)
    : _id(id), _position(position), _disposed(false)
{
    update();
}

void KDTreePartition::Adapter::Point::update()
{
    const V v2 = _position->val();
    _v[0] = v2.x();
    _v[1] = v2.y();
}

void KDTreePartition::Adapter::Point::dispose()
{
    _position = nullptr;
    _disposed = true;
}

KDTreePartition::KDTreePartition()
    : _id_generator(0), _updated_count(0)
{
    buildIndex();
}

uint32_t KDTreePartition::addPoint(const sp<VV>& pos)
{
    uint32_t id = _adapter.pts.size();
    const Adapter::Point pt(_id_generator++, pos);
    _adapter.pts.push_back(pt);
    _tree->addPoints(id, id);
    _id_map[pt._id] = id;
    return pt._id;
}

void KDTreePartition::updatePoint(uint32_t id)
{
    getPoint(id).update();
    rebuildIfNeeded();
}

void KDTreePartition::removePoint(uint32_t id)
{
    const auto iter = _id_map.find(id);
    DCHECK(iter != _id_map.end(), "ID: %d does not exists", id);
    uint32_t idx = iter->second;
    _adapter.pts[idx].dispose();
    if(rebuildIfNeeded())
    {
        _tree->removePoint(idx);
        _id_map.erase(iter);
    }
}

List<uint32_t> KDTreePartition::within(float x, float y, float radius)
{
	List<uint32_t> result;
	const float pt[] = {x, y};
	std::vector<std::pair<uint32_t, float>> indices_dists;
	nanoflann::RadiusResultSet<float, uint32_t> resultSet(radius * radius, indices_dists);

	_tree->findNeighbors(resultSet, pt, nanoflann::SearchParams());

	for(const std::pair<uint32_t,float>& i : indices_dists)
	{
		uint32_t id = i.first;
		result.push_back(_adapter.pts[id]._id);
	}

	return result;
}

KDTreePartition::Adapter::Point& KDTreePartition::getPoint(uint32_t id)
{
    const auto iter = _id_map.find(id);
    DCHECK(iter != _id_map.end(), "ID: %d does not exists", id);
    uint32_t idx = iter->second;
    DCHECK(idx < _adapter.pts.size(), "Index out bounds, index: %d", idx);
    return _adapter.pts[idx];
}

void KDTreePartition::buildIndex()
{
    const std::vector<Adapter::Point> pts = std::move(_adapter.pts);
    _id_map.clear();
    _tree.reset(new kd_tree_t(2, _adapter, nanoflann::KDTreeSingleIndexAdaptorParams()));
    if(pts.size() > 0)
    {
        for(const Adapter::Point& i : pts)
        {
            if(!i._disposed)
            {
                _id_map[i._id] = _adapter.pts.size();
                _adapter.pts.push_back(i);
            }
        }
        _tree->addPoints(0, _adapter.pts.size() - 1);
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

KDTreePartition::BUILDER::BUILDER()
{
}

sp<Partition> KDTreePartition::BUILDER::build(const sp<Scope>& /*args*/)
{
    return sp<Partition>::adopt(new KDTreePartition());
}

}
