#ifndef ARK_CORE_EPI_VISIBILITY_H_
#define ARK_CORE_EPI_VISIBILITY_H_

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/inf/builder.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

namespace ark {

//[[script::bindings::extends(Boolean)]]
class ARK_API Visibility : public Boolean {
public:
//  [[script::bindings::auto]]
    Visibility(bool visible = false);
//  [[script::bindings::auto]]
    Visibility(const sp<Boolean>& visible);

    virtual bool val() override;

//  [[script::bindings::auto]]
    void show();
//  [[script::bindings::auto]]
    void hide();
//  [[script::bindings::auto]]
    void set(bool visible);
//  [[script::bindings::auto]]
    void set(const sp<Boolean>& visible);

    const sp<Boolean>& delegate() const;

//  [[plugin::builder::by-value]]
    class DICTIONARY : public Builder<Visibility> {
    public:
        DICTIONARY(BeanFactory& factory, const String& value);

        virtual sp<Visibility> build(const Scope& args);

    private:
        sp<Builder<Boolean>> _visible;
    };

protected:
    sp<BooleanWrapper> _visible;

};

}

#endif
