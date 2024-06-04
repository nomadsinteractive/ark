#pragma once

#include "core/base/api.h"
#include "core/base/with_timestamp.h"
#include "core/inf/updatable.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/base/v4.h"

#include "app/forwarding.h"

namespace ark {

class ARK_API Layout : public Updatable {
public:
    class ARK_API Node {
    public:
        Node(sp<LayoutParam> layoutParam, sp<ViewHierarchy> viewHierarchy, void* tag = nullptr);

        float contentWidth() const;
        float contentHeight() const;

        float occupyWidth() const;
        float occupyHeight() const;

        const V4& paddings() const;
        void setPaddings(const V4& paddings);

        const V4& margins() const;
        void setMargins(const V4& margins);

        const WithTimestamp<V2>& offsetPosition() const;
        void setOffsetPosition(const V2& offsetPosition);

        const WithTimestamp<V2>& size() const;
        void setSize(const V2& size);

        sp<LayoutParam> _layout_param;
        sp<ViewHierarchy> _view_hierarchy;
        void* _tag;

    private:
        V4 _paddings;
        V4 _margins;
        WithTimestamp<V2> _offset_position;
        WithTimestamp<V2> _size;
    };

public:
    ~Layout() override = default;

    virtual void inflate(sp<Node> rootNode) = 0;
};

}
