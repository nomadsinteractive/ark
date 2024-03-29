#ifndef ARK_APP_INF_LAYOUT_H_
#define ARK_APP_INF_LAYOUT_H_

#include <vector>

#include "core/base/api.h"
#include "core/base/with_timestamp.h"
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
    class ARK_API Node {
    public:
        Node(sp<LayoutParam> layoutParam, sp<ViewHierarchy> viewHierarchy, void* tag = nullptr);

        float contentWidth() const;
        float contentHeight() const;

        const V4& paddings() const;
        void setPaddings(const V4& paddings);

        const WithTimestamp<V2>& offsetPosition() const;
        void setOffsetPosition(const V2& offsetPosition);

        const WithTimestamp<V2>& size() const;
        void setSize(const V2& size);

        sp<LayoutParam> _layout_param;
        sp<ViewHierarchy> _view_hierarchy;
        void* _tag;

    private:
        V4 _paddings;
        WithTimestamp<V2> _offset_position;
        WithTimestamp<V2> _size;
    };

public:
    virtual ~LayoutV3() = default;

    virtual void inflate(sp<Node> rootNode) = 0;
};

}

#endif
