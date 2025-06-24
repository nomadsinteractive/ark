#include "core/base/timestamp.h"

#include "core/ark.h"

#include <limits>

#include "app/base/application_context.h"

namespace ark {

Timestamp::Timestamp()
    : _last_modified(std::numeric_limits<uint64_t>::max())
{
}

bool Timestamp::update(const uint64_t timestamp) const
{
    if(_last_modified >= timestamp)
    {
        _last_modified = timestamp;
        return true;
    }
    return false;
}

void Timestamp::markClean()
{
    _last_modified = 0;
}

void Timestamp::markDirty()
{
    _last_modified = std::numeric_limits<uint64_t>::max();
}

uint64_t Timestamp::now()
{
    return Ark::instance().applicationContext()->timestamp();
}

}
