#ifndef ARK_CORE_EPI_DISPOSED_H_
#define ARK_CORE_EPI_DISPOSED_H_

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/inf/builder.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

namespace ark {

//[[script::bindings::extends(Boolean)]]
class ARK_API Disposed : public Boolean {
public:
//  [[script::bindings::auto]]
    Disposed(bool disposed = false);
//  [[script::bindings::auto]]
    Disposed(sp<Boolean> disposed);

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
    sp<BooleanWrapper> _disposed;

};

}

#endif
