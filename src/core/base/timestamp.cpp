#include "core/base/timestamp.h"

#include "core/ark.h"

#include <limits>

#include "app/base/application_context.h"

namespace ark {

Timestamp::Timestamp()
    : _last_modified(std::numeric_limits<uint32_t>::max())
{
}

bool Timestamp::update(const uint32_t tick) const
{
    if(_last_modified == tick)
        return true;
    if(_last_modified > tick)
    {
        _last_modified = tick;
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
    _last_modified = std::numeric_limits<uint32_t>::max();
}

uint32_t Timestamp::now()
{
    return Ark::instance().applicationContext()->tick();
}

}
