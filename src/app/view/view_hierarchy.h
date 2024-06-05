#pragma once

#include <vector>

#include "core/forwarding.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/base/v3.h"
#include "graphics/inf/layout.h"

#include "app/forwarding.h"

namespace ark {

class ARK_API ViewHierarchy {
public:
    ViewHierarchy(sp<Layout> layout);

    bool isIsolatedLayout() const;

    bool updateDescendantLayout(uint64_t timestamp);
    bool updateLayout(const sp<Layout::Node>& layoutNode, uint64_t timestamp, bool isDirty);

    const std::vector<sp<View>>& updateSlots();

    void addView(sp<View> view);

private:
    bool updateHierarchy();

    Layout::Hierarchy toLayoutHierarchy(sp<Layout::Node> layoutNode) const;

    std::vector<sp<View>> getLayoutItems() const;

private:
    sp<Layout> _layout;
    sp<Updatable> _updatable_layout;

    std::vector<sp<View>> _slots;
    std::vector<sp<View>> _incremental;
};

}
