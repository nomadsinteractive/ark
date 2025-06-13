#pragma once

#include "core/base/api.h"
#include "core/base/with_timestamp.h"
#include "core/inf/updatable.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/base/v4.h"

namespace ark {

class ARK_API Layout {
public:
    class ARK_API Node {
    public:
        Node(sp<LayoutParam> layoutParam, void* tag = nullptr);

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

        const WithTimestamp<float>& autoWidth() const;
        void setAutoWidth(float autoWidth);
        const WithTimestamp<float>& autoHeight() const;
        void setAutoHeight(float autoHeight);

        bool update(uint32_t timestamp);

        sp<LayoutParam> _layout_param;
        void* _tag;

    private:
        V4 _paddings;
        V4 _margins;
        WithTimestamp<V2> _offset_position;
        WithTimestamp<V2> _size;
        WithTimestamp<float> _auto_width;
        WithTimestamp<float> _auto_height;
    };

    struct Hierarchy {
        sp<Node> _node;
        Vector<Hierarchy> _child_nodes;
    };

public:
    virtual ~Layout() = default;

    virtual sp<Updatable> inflate(Hierarchy hierarchy) = 0;
};

}
