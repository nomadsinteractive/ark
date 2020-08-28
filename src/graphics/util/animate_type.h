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
    static sp<Animate> create(const sp<Animate>& value);

//[[script::bindings::classmethod]]
    static void set(const sp<Animate>& self, const sp<Animate>& delegate);

private:
    class AnimateWrapper : public Animate, Implements<AnimateWrapper, Animate> {
    public:
        AnimateWrapper(sp<Animate> delegate);

        virtual bool update(uint64_t timestamp) override;

        virtual void flat(void* buf) override;

        virtual uint32_t size() override;

        void setDelegate(sp<Animate> delegate);

    private:
        sp<Animate> _delegate;
    };

private:
    static sp<AnimateWrapper> ensureImpl(const sp<Animate>& self);

};

}

#endif
