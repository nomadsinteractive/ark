#pragma once

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/inf/builder.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

namespace ark {

//[[script::bindings::extends(Boolean)]]
class ARK_API Disposed final : public Boolean {
public:
//  [[script::bindings::auto]]
    Disposed(bool discarded = false);
//  [[script::bindings::auto]]
    Disposed(sp<Boolean> discarded);

    virtual bool val() override;
    virtual bool update(uint64_t timestamp) override;

//  [[script::bindings::auto]]
    void dispose();

//  [[script::bindings::auto]]
    void set(bool disposed);
//  [[script::bindings::auto]]
    void set(sp<Boolean> disposed);

    const sp<Boolean>& wrapped() const;

//  [[plugin::builder::by-value]]
    class DICTIONARY : public Builder<Disposed> {
    public:
        DICTIONARY(BeanFactory& factory, const String& value);

        virtual sp<Disposed> build(const Scope& args);

    private:
        bool _disposed;
        sp<Builder<Boolean>> _delegate;
    };

protected:
    sp<BooleanWrapper> _discarded;

};

}
