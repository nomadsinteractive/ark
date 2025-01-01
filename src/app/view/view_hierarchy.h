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

    bool isLayoutTopView() const;

    bool updateDescendantLayout(uint64_t timestamp);
    bool updateLayout(const sp<Layout::Node>& layoutNode, uint64_t timestamp, bool layoutParamDirty);

    const std::vector<sp<View>>& updateChildren();
    void markHierarchyDirty();

    void addView(sp<View> view);

private:
    bool updateHierarchy();

    Layout::Hierarchy toLayoutHierarchy(sp<Layout::Node> layoutNode) const;

private:
    sp<Layout> _layout;
    sp<Updatable> _updatable_layout;

    std::vector<sp<View>> _children;
    std::vector<sp<View>> _incremental;

    Timestamp _timestamp;
};

}
