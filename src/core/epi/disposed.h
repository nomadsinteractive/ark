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
    Disposed(const sp<Boolean>& disposed);

    virtual bool val() override;

//  [[script::bindings::auto]]
    void dispose();

//  [[script::bindings::auto]]
    void set(bool disposed);
//  [[script::bindings::auto]]
    void set(const sp<Boolean>& disposed);

    const sp<Boolean>& delegate() const;

//  [[plugin::builder::by-value]]
    class DICTIONARY : public Builder<Disposed> {
    public:
        DICTIONARY(BeanFactory& factory, const String& value);

        virtual sp<Disposed> build(const sp<Scope>& args);

    private:
        bool _disposed;
        sp<Builder<Boolean>> _delegate;
    };

protected:
    sp<BooleanWrapper> _disposed;

};

}

#endif
