#ifndef ARK_GRAPHICS_UTIL_ANIMATE_TYPE_H_
#define ARK_GRAPHICS_UTIL_ANIMATE_TYPE_H_

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/inf/flatable.h"
#include "core/types/implements.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

namespace ark {

//[[script::bindings::class("Animate")]]
class ARK_API AnimateType {
public:
//[[script::bindings::constructor]]
    static sp<Flatable> create(const sp<Flatable>& value);

//[[script::bindings::classmethod]]
    static void set(const sp<Flatable>& self, const sp<Flatable>& delegate);
//[[script::bindings::property]]
    static uint32_t size(const sp<Flatable>& self);

private:
    class AnimateWrapper : public Flatable, Implements<AnimateWrapper, Flatable> {
    public:
        AnimateWrapper(sp<Flatable> delegate);

        virtual bool update(uint64_t timestamp) override;

        virtual void flat(void* buf) override;

        virtual uint32_t size() override;

        void setDelegate(sp<Flatable> delegate);

    private:
        sp<Flatable> _delegate;
    };

private:
    static sp<AnimateWrapper> ensureImpl(const sp<Flatable>& self);

};

typedef Flatable Animate;

}

#endif
