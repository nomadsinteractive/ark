#pragma once

#include "core/types/weak_ptr.h"

#include "app/forwarding.h"

namespace ark {

class GraphRoute {
public:
    GraphRoute(GraphNode& entry, GraphNode& exit, float length);

    GraphNode& entry();
    GraphNode& exit();

    float length() const;
    bool isExternal() const;

private:
    GraphNode* _entry;
    GraphNode* _exit;
    float _length;

    bool _external;
};

}

