#pragma once

#include "core/forwarding.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/base/v3.h"
#include "graphics/inf/layout.h"

#include "app/forwarding.h"

namespace ark {

class ARK_API ViewHierarchy {
public:
    ViewHierarchy(sp<Layout> layout, sp<Layout::Node> layoutNode);

    bool isLayoutTopView() const;
    const sp<Layout>& layout() const;

    bool updateDescendantLayout(uint32_t tick);
    bool updateLayout(uint32_t tick);

    const Vector<sp<View>>& updateChildren();
    void markHierarchyDirty();

    void addView(sp<View> view);

    Layout::Hierarchy toLayoutHierarchy() const;

private:
    bool updateHierarchy();

private:
    sp<Layout> _layout;
    sp<Layout::Node> _layout_node;
    sp<Updatable> _updatable_layout;

    Vector<sp<View>> _children;
    Vector<sp<View>> _incremental;

    Timestamp _timestamp;
};

}
