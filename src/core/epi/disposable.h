#ifndef ARK_CORE_EPI_DISPOSABLE_H_
#define ARK_CORE_EPI_DISPOSABLE_H_

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/inf/builder.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

namespace ark {

class ARK_API Disposable {
public:
//  [[script::bindings::auto]]
    Disposable(bool disposed = false);
//  [[script::bindings::auto]]
    Disposable(const sp<Boolean>& disposed);

    sp<Boolean> toBoolean() const;

    bool isDisposed() const;

//  [[script::bindings::auto]]
    void dispose();

//  [[plugin::builder::by-value]]
    class DICTIONARY : public Builder<Disposable> {
    public:
        DICTIONARY(BeanFactory& parent, const String& value);

        virtual sp<Disposable> build(const sp<Scope>& args);

    private:
        bool _disposed;
        sp<Builder<Boolean>> _delegate;
    };

protected:
    sp<BooleanWrapper> _disposed;

};

}

#endif
