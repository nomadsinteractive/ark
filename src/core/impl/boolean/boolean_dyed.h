#ifndef ARK_CORE_IMPL_BOOLEAN_DYED_H_
#define ARK_CORE_IMPL_BOOLEAN_DYED_H_

#include "core/forwarding.h"
#include "core/base/wrapper.h"
#include "core/base/string.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

namespace ark {

class BooleanDyed : public Wrapper<Boolean>, public Boolean {
public:
    BooleanDyed(sp<Boolean> delegate, sp<Boolean> condition, String message);

    virtual bool update(uint64_t timestamp) override;
    virtual bool val() override;

private:
    sp<Boolean> _condition;
    String _message;

};

}

#endif
