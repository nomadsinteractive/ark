#ifndef ARK_CORE_IMPL_INTEGER_INTEGER_WRAPPER_H_
#define ARK_CORE_IMPL_INTEGER_INTEGER_WRAPPER_H_

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

namespace ark {

class ARK_API IntegerWrapper : public Integer {
public:
    IntegerWrapper(const sp<Integer>& delegate);
    IntegerWrapper(int32_t value);

    virtual int32_t val() override;

    const sp<Integer>& delegate() const;

    void set(int32_t value);
    void set(const sp<Integer>& delegate);

    void fix();

private:
    void deferedUnref();

private:
    sp<Integer> _delegate;
    int32_t _value;
};

}

#endif
