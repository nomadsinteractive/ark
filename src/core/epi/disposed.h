#ifndef ARK_CORE_EPI_DISPOSED_H_
#define ARK_CORE_EPI_DISPOSED_H_

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"

namespace ark {

class ARK_API Disposed {
public:
//  [[script::bindings::auto]]
    Disposed(bool disposed = false);
//  [[script::bindings::auto]]
    Disposed(const sp<Boolean>& disposed);

    sp<Boolean> toBoolean() const;

    bool isDisposed() const;

//  [[script::bindings::operator(bool)]]
    static bool __bool__(const sp<Disposed>& self);
//  [[script::bindings::operator(||)]]
    static sp<Boolean> __or__(const sp<Disposed>& lvalue, const sp<Disposed>& rvalue);
//  [[script::bindings::operator(||)]]
    static sp<Boolean> __or__(const sp<Disposed>& lvalue, const sp<Boolean>& rvalue);
//  [[script::bindings::operator(||)]]
    static sp<Boolean> __or__(const sp<Boolean>& lvalue, const sp<Disposed>& rvalue);
//  [[script::bindings::operator(&&)]]
    static sp<Boolean> __and__(const sp<Disposed>& lvalue, const sp<Disposed>& rvalue);
//  [[script::bindings::operator(&&)]]
    static sp<Boolean> __and__(const sp<Disposed>& lvalue, const sp<Boolean>& rvalue);
//  [[script::bindings::operator(&&)]]
    static sp<Boolean> __and__(const sp<Boolean>& lvalue, const sp<Disposed>& rvalue);

//  [[script::bindings::auto]]
    void dispose();

//  [[script::bindings::auto]]
    void set(bool disposed);
//  [[script::bindings::auto]]
    void set(const sp<Boolean>& disposed);

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
