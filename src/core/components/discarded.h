#pragma once

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/inf/builder.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

namespace ark {

//[[script::bindings::extends(Boolean)]]
class ARK_API Discarded final : public Boolean {
public:
//  [[script::bindings::auto]]
    Discarded(bool discarded = false);
//  [[script::bindings::auto]]
    Discarded(sp<Boolean> discarded);

    bool val() override;
    bool update(uint64_t timestamp) override;

//  [[script::bindings::auto]]
    void discard();

//  [[script::bindings::auto]]
    void set(bool discarded);
//  [[script::bindings::auto]]
    void set(sp<Boolean> discarded);

//  [[plugin::builder::by-value]]
    class DICTIONARY final : public Builder<Discarded> {
    public:
        DICTIONARY(BeanFactory& factory, const String& value);

        sp<Discarded> build(const Scope& args) override;

    private:
        bool _discarded;
        sp<Builder<Boolean>> _delegate;
    };

private:
    sp<BooleanWrapper> _discarded;
};

}
