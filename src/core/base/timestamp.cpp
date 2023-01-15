#include "core/base/timestamp.h"

#include <limits>


namespace ark {

Timestamp::Timestamp()
    : _last_modified(0)
{
}

bool Timestamp::update(uint64_t timestamp)
{
    if(_last_modified >= timestamp)
    {
        if(_last_modified == std::numeric_limits<uint64_t>::max())
            _last_modified = timestamp;
        return true;
    }
    return false;
}

void Timestamp::markDirty()
{
    _last_modified = std::numeric_limits<uint64_t>::max();
}

}
