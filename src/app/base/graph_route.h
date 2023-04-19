#pragma once

#include "core/types/weak_ptr.h"

#include "app/forwarding.h"

namespace ark {

class GraphRoute {
public:
    GraphRoute(GraphNode& entry, GraphNode& exit, float weight);

    GraphNode& entry();
    GraphNode& exit();

    float weight() const;
    bool isExternal() const;

private:
    GraphNode* _entry;
    GraphNode* _exit;
    float _weight;

    bool _external;
};

}

