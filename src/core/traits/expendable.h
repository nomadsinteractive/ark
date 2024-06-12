#pragma once

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/inf/builder.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

namespace ark {

//[[script::bindings::extends(Boolean)]]
class ARK_API Expendable final : public Boolean {
public:
//  [[script::bindings::auto]]
    Expendable(bool discarded = false);
//  [[script::bindings::auto]]
    Expendable(sp<Boolean> discarded);

    bool val() override;
    bool update(uint64_t timestamp) override;

//  [[script::bindings::auto]]
    void discard();

//  [[script::bindings::auto]]
    void set(bool disposed);
//  [[script::bindings::auto]]
    void set(sp<Boolean> disposed);

//  [[plugin::builder::by-value]]
    class DICTIONARY : public Builder<Expendable> {
    public:
        DICTIONARY(BeanFactory& factory, const String& value);

        sp<Expendable> build(const Scope& args) override;

    private:
        bool _discarded;
        sp<Builder<Boolean>> _delegate;
    };

private:
    sp<BooleanWrapper> _discarded;

};

}
