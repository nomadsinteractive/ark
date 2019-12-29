#include "renderer/inf/vertices.h"

namespace ark {

Vertices::Vertices(size_t length)
    : _length(length)
{
}

size_t Vertices::length() const
{
    return _length;
}

}
