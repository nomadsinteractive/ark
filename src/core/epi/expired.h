#ifndef ARK_CORE_EPI_EXPIRED_H_
#define ARK_CORE_EPI_EXPIRED_H_

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/inf/builder.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

namespace ark {

//[[core::class]]
class ARK_API Expired : public Boolean {
public:
//  [[script::bindings::auto]]
    Expired(bool expired = false);
    Expired(const sp<Boolean>& delegate);

    virtual bool val() override;

//  [[script::bindings::auto]]
    bool expired() const;
//  [[script::bindings::auto]]
    void expire();

//  [[plugin::builder]]
    class BUILDER : public Builder<Expired> {
    public:
        BUILDER(BeanFactory& parent, const document& doc);

        virtual sp<Expired> build(const sp<Scope>& args) override;

    private:
        sp<Builder<Boolean>> _delegate;
    };

//  [[plugin::builder::by-value]]
    class DICTIONARY : public Builder<Expired> {
    public:
        DICTIONARY(BeanFactory& parent, const String& value);

        virtual sp<Expired> build(const sp<Scope>& args);

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
