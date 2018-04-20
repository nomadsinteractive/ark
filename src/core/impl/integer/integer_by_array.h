#ifndef ARK_CORE_IMPL_INTEGER_INTEGER_REPEATER_H_
#define ARK_CORE_IMPL_INTEGER_INTEGER_REPEATER_H_

#include "core/forwarding.h"
#include "core/inf/builder.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

namespace ark {

class IntegerByArray : public Integer {
public:
    enum Repeat {
        REPEAT_NONE,
        REPEAT_RESTART,
        REPEAT_REVERSE
    };

    struct Stub {
        Stub(const sp<IntArray>& array, Repeat repeat);

        int32_t val();

        sp<IntArray> _array;
        int32_t _position;
        Repeat _repeat;
        int32_t _step;
    };

    IntegerByArray(const sp<IntArray>& array, Repeat repeat);

    virtual int32_t val() override;

//  [[plugin::builder("by-array")]]
    class BUILDER : public Builder<Integer> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        virtual sp<Integer> build(const sp<Scope>& args) override;

    private:
        sp<Builder<IntArray>> _array;
        Repeat _repeat;
    };

private:
    sp<Stub> _stub;

    friend class BUILDER;
};

}

#endif
