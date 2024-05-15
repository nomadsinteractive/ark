#pragma once

#include <vector>

#include "core/forwarding.h"
#include "core/inf/holder.h"
#include "core/inf/updatable.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/base/v3.h"

#include "app/forwarding.h"
#include "app/inf/layout.h"

namespace ark {

class ARK_API ViewHierarchy : public Holder {
public:
    class ARK_API Slot : public Holder {
    public:
        Slot(sp<View> view);

        virtual void traverse(const Visitor& visitor) override;

        bool isDiscarded() const;
        bool isVisible() const;

        void updateLayout();
        void updateLayoutPosition(const V2& position);

        const sp<LayoutParam>& layoutParam() const;
        const sp<LayoutV3::Node>& layoutNode() const;

    private:
        sp<View> _view;

        friend class ViewHierarchy;
    };

public:
    ViewHierarchy(sp<Layout> layout, sp<LayoutV3> layoutV3);

    virtual void traverse(const Visitor& visitor) override;

    bool isIsolatedLayout() const;

    bool updateDescendantLayout(uint64_t timestamp);
    bool updateLayout(const LayoutParam& layoutParam, const sp<LayoutV3::Node>& layoutNode, uint64_t timestamp, bool isDirty);

    const std::vector<sp<Slot>>& updateSlots();

    void addView(sp<View> view);

private:
    bool isInflateNeeded();

    std::pair<std::vector<sp<Slot>>, std::vector<sp<LayoutParam>>> getLayoutItems() const;

private:
    sp<Layout> _layout;
    sp<LayoutV3> _layout_v3;

    std::vector<sp<Slot>> _slots;
    std::vector<sp<Slot>> _incremental;
};

}
