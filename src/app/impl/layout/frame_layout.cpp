#include "app/impl/layout/frame_layout.h"

#include "core/types/weak_ptr.h"
#include "core/util/numeric_util.h"
#include "core/util/variable_util.h"

#include "app/view/layout_param.h"
#include "app/impl/layout/gravity_layout.h"

namespace ark {

namespace  {

class FramePosition : public Numeric {
public:
    FramePosition(const sp<LayoutNew::Slot>& slot)
        : _slot(slot) {
    }

    virtual float val() override {
        return _slot.ensure()->_margin_before.val();
    }

    virtual bool update(uint64_t timestamp) override {
        return _slot.ensure()->_margin_before.update(timestamp);
    }

private:
    WeakPtr<LayoutNew::Slot> _slot;
};

class FramePositionWithGravity : public Numeric {
public:
    FramePositionWithGravity(const WeakPtr<LayoutNew::Slot>& parentSize, const sp<LayoutNew::Slot>& slot)
        : _parent(parentSize), _slot(slot) {
    }

    virtual float val() override {
        return _val;
    }

    virtual bool update(uint64_t timestamp) override {
        const sp<LayoutNew::Slot> slot = _slot.ensure();
        const sp<LayoutNew::Slot> parent = _parent.ensure();
        if(VariableUtil::update(timestamp, slot->_margin_before, slot->_margin_after, slot->_size, parent->_size)) {
            float size = slot->_size->val();
            _val = GravityLayout::place(parent->_gravity, size, parent->_size->val() - slot->_margin_before.val() - slot->_margin_after.val());
            return true;
        }
        return false;
    }

private:
    WeakPtr<LayoutNew::Slot> _parent;
    WeakPtr<LayoutNew::Slot> _slot;

    float _val;
};

class FrameSize : public Numeric {
public:
    FrameSize(const sp<Numeric>& parentSize, const sp<LayoutNew::Slot>& slot)
        : _parent_size(parentSize), _slot(slot) {
    }

    virtual float val() override {
        return _val;
    }

    virtual bool update(uint64_t timestamp) override {
        const sp<LayoutNew::Slot> slot = _slot.ensure();
        if(VariableUtil::update(timestamp, slot->_margin_before, slot->_margin_after, _parent_size)) {
            _val = _parent_size->val() - slot->_margin_before.val() - slot->_margin_after.val();
            return true;
        }
        return false;
    }

private:
    sp<Numeric> _parent_size;
    WeakPtr<LayoutNew::Slot> _slot;
    float _val;
};

}


void FrameLayout::begin(Context& /*ctx*/, LayoutParam& /*layoutParam*/)
{
}

Rect FrameLayout::place(Context& ctx, LayoutParam& layoutParam)
{
    return Rect(0, 0, layoutParam.calcLayoutWidth(ctx._client_width), layoutParam.calcLayoutHeight(ctx._client_height));
}

Rect FrameLayout::end(Context& /*ctx*/)
{
    return Rect(0, 0, 0, 0);
}

FrameLayout::BUILDER::BUILDER()
{
}

sp<Layout> FrameLayout::BUILDER::build(const Scope& /*args*/)
{
    return sp<Layout>::adopt(new FrameLayout());
}

std::vector<sp<Numeric>> FrameLayoutNew::place(const std::vector<sp<LayoutNew::Slot>>& children, const sp<Slot>& parent)
{
    std::vector<sp<Numeric>> slots;
    for(const sp<LayoutNew::Slot>& i : children)
    {
        if(i->_weight > 0)
            i->_size->set(sp<FrameSize>::make(parent->_size, i));
        slots.push_back(parent->_gravity != LayoutParam::NONE ? sp<Numeric>::make<FramePositionWithGravity>(parent, i) : sp<Numeric>::make<FramePosition>(i));
    }
    return slots;
}

}
