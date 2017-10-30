#include "app/impl/collider/simple_collider.h"

#include <algorithm>
#include <iterator>

#include "app/impl/collider/static_segments.h"

namespace ark {


StaticSegments::Segment::Segment(float position)
    : _position(position), _radius(0)
{
}

StaticSegments::Segment::Segment(uint32_t rigidBodyId, float position, float radius)
    : _id(rigidBodyId), _position(position), _radius(radius)
{
}

StaticSegments::Segment::Segment(const Segment& other)
    : _id(other._id), _position(other._position), _radius(other._radius)
{
}

bool StaticSegments::Segment::operator < (const Segment& other) const
{
    return _position < other._position;
}

StaticSegments::StaticSegments()
    : _search_radius(0)
{
}

void StaticSegments::insert(uint32_t id, float position, float radius)
{
    if(_search_radius < radius)
        _search_radius = radius;

    const Segment segment(id, position, radius);
    _blocks.insert(std::upper_bound(_blocks.begin(), _blocks.end(), segment), segment);
}

void StaticSegments::remove(uint32_t id)
{
    for(auto iter = _blocks.begin(); iter != _blocks.end(); ++iter)
    {
        const Segment& segment = *iter;
        if(segment._id == id)
        {
            if(segment._radius == _search_radius)
                updateSearchRadius();
            _blocks.erase(iter);
            break;
        }
    }
}

std::set<uint32_t> StaticSegments::findCandidates(float p1, float p2)
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

void StaticSegments::updateSearchRadius()
{
    _search_radius = 0;
    for(auto iter = _blocks.begin(); iter != _blocks.end(); ++iter)
        if(_search_radius < iter->_radius)
            _search_radius = iter->_radius;
}

}
