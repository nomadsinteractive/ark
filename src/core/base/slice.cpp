#include "core/base/slice.h"

namespace ark {

Slice::Slice(ptrdiff_t begin, ptrdiff_t end, ptrdiff_t step)
    : _begin(begin), _end(end), _step(step)
{
}

ptrdiff_t Slice::begin() const
{
    return _begin;
}

ptrdiff_t Slice::end() const
{
    return _end;
}

ptrdiff_t Slice::step() const
{
    return _step;
}

ptrdiff_t Slice::length() const
{
    return _end - _begin;
}

Slice Slice::adjustIndices(size_t length) const
{
    Slice adjusted = *this;
    if(_end == std::numeric_limits<ptrdiff_t>::max())
        adjusted._end = length;
    else if(_end < 0)
        adjusted._end = length + _end;
    if(_begin < 0)
        adjusted._begin = length + _begin;
    return adjusted;
}

}
