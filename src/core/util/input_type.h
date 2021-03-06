#ifndef ARK_CORE_UTIL_INPUT_TYPE_H_
#define ARK_CORE_UTIL_INPUT_TYPE_H_

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/inf/input.h"
#include "core/types/implements.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

namespace ark {

//[[script::bindings::class("Input")]]
class ARK_API InputType {
public:
//[[script::bindings::constructor]]
    static sp<Input> create(const sp<Input>& value);

//[[script::bindings::classmethod]]
    static void set(const sp<Input>& self, const sp<Input>& delegate);
//[[script::bindings::property]]
    static uint32_t size(const sp<Input>& self);

private:
    class InputWrapper : public Input, Implements<InputWrapper, Input> {
    public:
        InputWrapper(sp<Input> delegate);

        virtual bool update(uint64_t timestamp) override;

        virtual void flat(void* buf) override;

        virtual uint32_t size() override;

        void setDelegate(sp<Input> delegate);

    private:
        sp<Input> _delegate;
    };

private:
    static sp<InputWrapper> ensureImpl(const sp<Input>& self);

};

}

#endif
