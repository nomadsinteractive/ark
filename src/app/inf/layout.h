#ifndef ARK_APP_INF_LAYOUT_H_
#define ARK_APP_INF_LAYOUT_H_

#include <vector>

#include "core/base/api.h"
#include "core/inf/updatable.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/base/v4.h"

#include "app/forwarding.h"

namespace ark {

class ARK_API Layout {
public:
    virtual ~Layout() = default;

    virtual V2 inflate(const std::vector<sp<LayoutParam>>& slots) = 0;
    virtual std::vector<V2> place(const std::vector<sp<LayoutParam>>& slots, const LayoutParam& parent, const V2& contentSize) = 0;

};

class ARK_API LayoutV3 : public Updatable {
public:
    struct Node {
        Node(sp<LayoutParam> layoutParam, sp<ViewHierarchy> viewHierarchy, void* tag = nullptr)
            : _layout_param(std::move(layoutParam)), _view_hierarchy(std::move(viewHierarchy)), _tag(tag) {
        }

        sp<LayoutParam> _layout_param;
        sp<ViewHierarchy> _view_hierarchy;
        void* _tag;

        V4 _paddings;
        V2 _offset_position;
        V2 _size;
    };

public:
    virtual ~LayoutV3() = default;

    virtual void inflate(sp<Node> rootNode) = 0;
};

}

#endif
