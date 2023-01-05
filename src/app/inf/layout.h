#ifndef ARK_APP_INF_LAYOUT_H_
#define ARK_APP_INF_LAYOUT_H_

#include <vector>

#include "core/base/api.h"

#include "graphics/forwarding.h"
#include "graphics/base/size.h"
#include "graphics/impl/vec/vec3_impl.h"

#include "app/forwarding.h"

namespace ark {

class ARK_API Layout {
public:
    virtual ~Layout() = default;

    virtual V2 inflate(const std::vector<sp<LayoutParam>>& slots) = 0;
    virtual std::vector<V2> place(const std::vector<sp<LayoutParam>>& slots, const LayoutParam& parent, const V2& contentSize) = 0;

};

class ARK_API LayoutV3 {
public:
    struct Node {
        Node(sp<LayoutParam> layoutParam, void* tag = nullptr)
            : _layout_param(std::move(layoutParam)), _position(sp<Vec3Impl>::make()), _size(sp<Size>::make()), _tag(tag) {
        }

        sp<LayoutParam> _layout_param;
        sp<Vec3Impl> _position;
        sp<Size> _size;
        void* _tag;

        std::vector<sp<Node>> _child_nodes;
    };

public:
    virtual ~LayoutV3() = default;

    virtual void inflate(Node& rootNode) = 0;
    virtual void place(Node& rootNode) = 0;
};

}

#endif
