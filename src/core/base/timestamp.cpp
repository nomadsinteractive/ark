#include "core/base/timestamp.h"

#include <stdint.h>
#include <limits>


namespace ark {

Timestamp::Timestamp()
    : _timestamp(0)
{
}

bool Timestamp::update(uint64_t timestamp)
{
    if(_timestamp >= timestamp)
    {
        _timestamp = timestamp;
        return true;
    }
    return false;
}

void Timestamp::setDirty()
{
    _timestamp = std::numeric_limits<uint64_t>::max();
}

}
