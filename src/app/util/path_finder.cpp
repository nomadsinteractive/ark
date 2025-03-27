#include "app/util/path_finder.h"

#include "app/base/graph_searching_node.h"
#include "core/components/behavior.h"

#include "graphics/base/v3.h"

#include "app/util/a_star.h"
#include "core/util/math.h"

namespace ark {

class PathFinder::SearchingNodeProviderImpl : public Graph::SearchingNodeProvider {
public:
    SearchingNodeProviderImpl(PathFinder& pathFinder, sp<Behavior::Method> method)
        : _path_finder(pathFinder), _method(std::move(method))
    {
    }

    void visitAdjacentNodes(const V3& position, const std::function<void(GraphSearchingNode, float)>& visitor) override
    {
        auto v = [this, position, visitor](const V3& pos) {
            const float weight = Math::distance(position, pos);
            visitor(GraphSearchingNode(pos, _path_finder._searching_node_provider), weight);
        };

        _method->call({
            Box(sp<Vec3>::make<Vec3::Const>(position)),
            Box(sp<std::function<void(const V3&)>>::make(std::move(v)))
        });
    }

private:
    PathFinder& _path_finder;
    sp<Behavior::Method> _method;
};

PathFinder::PathFinder(Behavior& behavior, const StringView name)
    : _searching_node_provider(sp<Graph::SearchingNodeProvider>::make<SearchingNodeProviderImpl>(*this, behavior.getMethod(name)))
{
}

Vector<V3> PathFinder::findPath(const V3& start, const V3& end) const
{
    Vector<V3> result;
    AStar<GraphSearchingNode> pathFinder(GraphSearchingNode(start, _searching_node_provider), GraphSearchingNode(end, _searching_node_provider));
    for(const GraphSearchingNode& i : pathFinder.findPath())
        result.push_back(i.position());
    return result;
}

}
