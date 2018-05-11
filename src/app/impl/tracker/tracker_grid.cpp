#include "app/impl/tracker/tracker_grid.h"

namespace ark {

void TrackerGrid::Axis::update(int32_t id, float pos, float radius)
{
    const auto iter = _trackee_grids.find(id);
    if(iter != _trackee_grids.end())
        _trackee_grids.erase(iter);

    int32_t begin = static_cast<int32_t>(pos );
    _trackee_grids.insert(std::make_pair(id, radius));
}

}
