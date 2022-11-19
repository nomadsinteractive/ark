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
    static sp<Input> create(sp<Numeric> value);
//[[script::bindings::constructor]]
    static sp<Input> create(std::vector<sp<Mat4>> value);
//[[script::bindings::constructor]]
    static sp<Input> create(std::vector<sp<Vec4>> value);
//[[script::bindings::constructor]]
    static sp<Input> create(std::vector<sp<Input>> value);
//[[script::bindings::constructor]]
    static sp<Input> create(std::vector<V3> value);
//[[script::bindings::constructor]]
    static sp<Input> create(std::vector<V4> value);
//[[script::bindings::constructor]]
    static sp<Input> create(std::vector<uint32_t> value);

//[[script::bindings::property]]
    static uint32_t size(const sp<Input>& self);

//[[script::bindings::classmethod]]
    static void set(const sp<Input>& self, const sp<Input>& delegate);
//[[script::bindings::classmethod]]
    static sp<Input> wrap(sp<Input> self);

//[[script::bindings::auto]]
    static sp<Input> makeElementIndexInput(std::vector<element_index_t> value);

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
