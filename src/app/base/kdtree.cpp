#include "app/base/kdtree.h"

#include <iostream>

#include <nanoflann.hpp>

#include "core/collection/list.h"
#include "core/inf/variable.h"

#include "graphics/base/v2.h"

namespace ark {

namespace {

struct Adapter {

    struct Point {
        Point(uint32_t id, const sp<Vec>& position)
            : _id(id), _position(position) {
            update();
        }
        Point(const Point& other) = default;

        void update() {
            const V v2 = _position->val();
            _v[0] = v2.x();
            _v[1] = v2.y();
        }

        uint32_t _id;
        sp<Vec> _position;
        float _v[2];
    };

    std::vector<Point> pts;

    inline size_t kdtree_get_point_count() const { return pts.size(); }
    inline float kdtree_get_pt(const size_t idx, int dim) const { return pts[idx]._v[dim]; }
    template <class BBOX> bool kdtree_get_bbox(BBOX& /* bb */) const { return false; }

};

typedef nanoflann::KDTreeSingleIndexDynamicAdaptor<
	nanoflann::L2_Simple_Adaptor<float, Adapter>,
	Adapter, 2, uint32_t> kd_tree_t;

}

struct KDTree::Stub {
    Stub(const nanoflann::KDTreeSingleIndexAdaptorParams& params)
        : _tree(2, _adapter, params) {
    }

    Adapter::Point& getPoint(uint32_t id) {
        DCHECK(id < _adapter.pts.size(), "Index out bounds, index: %d", id);
        return _adapter.pts[id];
    }

    Adapter _adapter;
    kd_tree_t _tree;
};

KDTree::KDTree()
    : _stub(new Stub(nanoflann::KDTreeSingleIndexAdaptorParams())), _updated_count(0)
{
    buildIndex();
}

uint32_t KDTree::addPoint(const sp<Vec>& pos)
{
    rebuildIfNeeded();
    if(_id_recycler.size() > 0)
    {
        const auto head = _id_recycler.begin();
        uint32_t recycledId = *head;
        _stub->_adapter.pts[recycledId] = Adapter::Point(recycledId, pos);
        _stub->_tree.addPoints(recycledId, recycledId);
        _id_recycler.erase(head);
        return recycledId;
    }
    uint32_t id = _stub->_adapter.pts.size();
    _stub->_adapter.pts.push_back(Adapter::Point(id, pos));
    _stub->_tree.addPoints(id, id);
    return id;
}

void KDTree::updatePoint(uint32_t id)
{
    _stub->getPoint(id).update();
}

void KDTree::removePoint(uint32_t id)
{
    DCHECK(_id_recycler.find(id) == _id_recycler.end(), "Id: %d has been deleted already", id);
    _stub->_tree.removePoint(id);
    _id_recycler.insert(id);
    rebuildIfNeeded();
}

std::unordered_set<uint32_t> KDTree::within(float x, float y, float radius)
{
	std::unordered_set<uint32_t> result;
	const float pt[] = {x, y};

    std::vector<std::pair<uint32_t, float>> indices_dists;
    nanoflann::RadiusResultSet<float, uint32_t> resultSet(radius * radius, indices_dists);
    _stub->_tree.findNeighbors(resultSet, pt, nanoflann::SearchParams());

    for(const std::pair<uint32_t, float>& i : indices_dists)
        result.insert(i.first);

	return result;
}

void KDTree::buildIndex()
{
    const std::vector<Adapter::Point> pts = std::move(_stub->_adapter.pts);
    _stub.reset(new Stub(nanoflann::KDTreeSingleIndexAdaptorParams()));
    if(pts.size() > 0)
    {
        _stub->_adapter.pts = std::move(pts);
        _stub->_tree.addPoints(0, _stub->_adapter.pts.size() - 1);
        for(uint32_t i : _id_recycler)
            _stub->_tree.removePoint(i);
    }
}

bool KDTree::rebuildIfNeeded()
{
    if(++_updated_count > _stub->_adapter.pts.size() && _stub->_adapter.pts.size() > 10)
    {
        buildIndex();
        _updated_count = 0;
        return false;
    }
    return true;
}

}
