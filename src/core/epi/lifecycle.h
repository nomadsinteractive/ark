#ifndef ARK_CORE_EPI_LIFECYCLE_H_
#define ARK_CORE_EPI_LIFECYCLE_H_

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/inf/builder.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

namespace ark {

class ARK_API Lifecycle : public Boolean {
public:
//  [[script::bindings::auto]]
    Lifecycle(bool expired = false);
//  [[script::bindings::auto]]
    Lifecycle(const sp<Boolean>& delegate);

    virtual bool val() override;

//  [[script::bindings::auto]]
    bool expired() const;
//  [[script::bindings::auto]]
    void expire();

//  [[plugin::builder::by-value]]
    class DICTIONARY : public Builder<Lifecycle> {
    public:
        DICTIONARY(BeanFactory& parent, const String& value);

        virtual sp<Lifecycle> build(const sp<Scope>& args);

    private:
        bool _expired;
        sp<Builder<Boolean>> _delegate;
    };

protected:
    bool _expired;
    sp<Boolean> _delegate;

};

}

#endif
