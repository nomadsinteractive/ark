#include "app/impl/collider/axis_segments.h"

#include "core/base/api.h"

#include <algorithm>
#include <iterator>

namespace ark {


AxisSegments::Segment::Segment(float position)
    : _position(position), _radius(0)
{
}

AxisSegments::Segment::Segment(uint32_t rigidBodyId, float position, float radius)
    : _id(rigidBodyId), _position(position), _radius(radius)
{
}

AxisSegments::Segment::Segment(const Segment& other)
    : _id(other._id), _position(other._position), _radius(other._radius)
{
}

bool AxisSegments::Segment::operator < (const Segment& other) const
{
    return _position < other._position;
}

void AxisSegments::Segment::update(float position, float radius)
{
    _position = position;
    _radius = radius;
}

AxisSegments::AxisSegments()
    : _search_radius(0)
{
}

void AxisSegments::insert(uint32_t id, float position, float radius)
{
    if(_search_radius < radius)
        _search_radius = radius;

    const Segment segment(id, position, radius);
    _blocks.insert(std::upper_bound(_blocks.begin(), _blocks.end(), segment), segment);
    _positions[id] = position;
}

void AxisSegments::update(uint32_t id, float position, float radius)
{
    const Segment segment(id, position, radius);
    auto iter = std::lower_bound(_blocks.begin(), _blocks.end(), segment);
    float lastRadius;
    if(iter == _blocks.end())
    {
        auto piter = findSegmentById(id);
        lastRadius = piter->_radius;
        _blocks.erase(findSegmentById(id));
        _blocks.push_back(segment);
    }
    else
    {
        if(iter->_id == id)
        {
            lastRadius = iter->_radius;
            iter->update(position, radius);
        }
        else
        {
            auto piter = findSegmentById(id);
            bool less = piter < iter;
            lastRadius = piter->_radius;
            piter = _blocks.erase(piter);
            if(less)
                _blocks.insert(std::upper_bound(piter, _blocks.end(), segment), segment);
            else
                _blocks.insert(std::upper_bound(_blocks.begin(), piter, segment), segment);
        }
    }
    if(radius >= _search_radius)
        _search_radius = radius;
    else if(lastRadius == _search_radius && radius < lastRadius)
        updateSearchRadius();
    _positions[id] = position;
}

void AxisSegments::remove(uint32_t id)
{
    const auto iter = findSegmentById(id);
    float radius = iter->_radius;
    _blocks.erase(iter);
    if(radius == _search_radius)
        updateSearchRadius();
    _positions.erase(id);
}

std::set<uint32_t> AxisSegments::findCandidates(float p1, float p2) const
{
    if(p1 > p2)
        std::swap(p1, p2);

    const auto start = std::lower_bound(_blocks.begin(), _blocks.end(), Segment(p1 - _search_radius));
    const auto end = std::upper_bound(_blocks.begin(), _blocks.end(), Segment(p2 + _search_radius));

    std::set<uint32_t> candidates;
    for(auto iter = start; iter != end; ++iter)
        candidates.insert(iter->_id);

    return candidates;
}

void AxisSegments::updateSearchRadius()
{
    _search_radius = 0;
    for(auto iter = _blocks.begin(); iter != _blocks.end(); ++iter)
        if(_search_radius < iter->_radius)
            _search_radius = iter->_radius;
}

std::vector<AxisSegments::Segment>::iterator AxisSegments::findSegmentById(uint32_t id)
{
    auto piter = _positions.find(id);
    DCHECK(piter != _positions.end(), "RigidBody(%d) not found.", id);
    const float position = piter->second;
    for(auto iter = std::lower_bound(_blocks.begin(), _blocks.end(), Segment(position)); iter != _blocks.end(); ++iter)
    {
        if(iter->_id == id)
            return iter;
        DCHECK(iter->_position == position, "RigidBody(%d) not found.", id);
    }
    return _blocks.end();
}

}
