#pragma once

#include <vector>

#include "core/forwarding.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/base/v3.h"

#include "app/forwarding.h"
#include "app/inf/layout.h"

namespace ark {

class ARK_API ViewHierarchy {
public:
    class ARK_API Slot {
    public:
        Slot(sp<View> view);

        bool isDiscarded() const;
        bool isVisible() const;

        void updateLayout();
        void updateLayoutPosition(const V2& position);

        const sp<LayoutParam>& layoutParam() const;
        const sp<Layout::Node>& layoutNode() const;

    private:
        sp<View> _view;

        friend class ViewHierarchy;
    };

public:
    ViewHierarchy(sp<Layout> layout);

    bool isIsolatedLayout() const;

    bool updateDescendantLayout(uint64_t timestamp);
    bool updateLayout(const sp<Layout::Node>& layoutNode, uint64_t timestamp, bool isDirty);

    const std::vector<sp<Slot>>& updateSlots();
    const std::vector<sp<Slot>>& updateSlotsAndLayoutNodes();

    void addView(sp<View> view);

private:
    bool isInflateNeeded();

    std::pair<std::vector<sp<Slot>>, std::vector<sp<LayoutParam>>> getLayoutItems() const;

private:
    sp<Layout> _layout;

    std::vector<sp<Slot>> _slots;
    std::vector<sp<Slot>> _incremental;
};

}
