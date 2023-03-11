#include "app/base/graph_route.h"

namespace ark {

GraphRoute::GraphRoute(GraphNode& entry, GraphNode& exit, float length)
    : _entry(entry), _exit(exit), _length(length)
{
}

GraphNode& GraphRoute::entry()
{
    return _entry;
}

GraphNode& GraphRoute::exit()
{
    return _exit;
}

float GraphRoute::length() const
{
    return _length;
}

}
