#include "app/util/path_finder.h"

#include "app/base/graph_searching_node.h"
#include "core/components/behavior.h"

#include "graphics/base/v3.h"

#include "app/util/a_star.h"

namespace ark {

namespace {

class SearchingNodeProviderImpl : public Graph::SearchingNodeProvider {
public:
    SearchingNodeProviderImpl(sp<Behavior::Method> method)
        : _method(std::move(method))
    {
    }

    void visitAdjacentNodes(const V3& position, const std::function<void(GraphSearchingNode, float)>& visitor) override
    {
        _method->call({
            Box(sp<Vec3>::make<Vec3::Const>(position)),
            Box(sp<std::function<void(GraphSearchingNode, float)>>::make(visitor))
        });
    }

private:
    sp<Behavior::Method> _method;
};

}


PathFinder::PathFinder(Behavior& behavior, const StringView visitAdjacentNodes)
    : _searching_node_provider(sp<Graph::SearchingNodeProvider>::make<SearchingNodeProviderImpl>(behavior.getMethod(visitAdjacentNodes)))
{
}

Vector<V3> PathFinder::findPath(const V3& start, const V3& end)
{
    Vector<V3> result;
    AStar<GraphSearchingNode> pathFinder(GraphSearchingNode(start, _searching_node_provider), GraphSearchingNode(end, _searching_node_provider));
    for(const GraphSearchingNode& i : pathFinder.findPath())
        result.push_back(i.position());
    return result;
}

sp<GraphSearchingNode> PathFinder::makeGraphSearchingNode(const V3& position)
{
    return sp<GraphSearchingNode>::make(position, _searching_node_provider);
}

}
