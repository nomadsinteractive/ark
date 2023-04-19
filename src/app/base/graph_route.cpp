#include "app/base/graph_route.h"

#include "app/base/graph.h"
#include "app/base/graph_node.h"

namespace ark {

GraphRoute::GraphRoute(GraphNode& entry, GraphNode& exit, float weight)
    : _entry(&entry), _exit(&exit), _weight(weight), _external(!entry.graph().hasNode(exit))
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

float GraphRoute::weight() const
{
    return _weight;
}

bool GraphRoute::isExternal() const
{
    return _external;
}

}
