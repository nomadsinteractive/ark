#include "app/base/graph_route.h"

#include "app/base/graph.h"
#include "app/base/graph_node.h"

namespace ark {

GraphRoute::GraphRoute(GraphNode& entry, GraphNode& exit, float length)
    : _entry(&entry), _exit(&exit), _length(length), _external(!entry.graph().hasNode(exit))
{
}

GraphNode& GraphRoute::entry()
{
    return *_entry;
}

GraphNode& GraphRoute::exit()
{
    return *_exit;
}

float GraphRoute::length() const
{
    return _length;
}

bool GraphRoute::isExternal() const
{
    return _external;
}

}
