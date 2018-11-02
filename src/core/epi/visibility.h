#ifndef ARK_CORE_EPI_VISIBILITY_H_
#define ARK_CORE_EPI_VISIBILITY_H_

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/inf/builder.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

namespace ark {

class ARK_API Visibility {
public:
//  [[script::bindings::auto]]
    Visibility(bool visible = false);
//  [[script::bindings::auto]]
    Visibility(const sp<Boolean>& visible);

    sp<Boolean> toBoolean() const;

//  [[script::bindings::property]]
    bool visible() const;
//  [[script::bindings::property]]
    void setVisible(bool visible);

//  [[plugin::builder::by-value]]
    class DICTIONARY : public Builder<Visibility> {
    public:
        DICTIONARY(BeanFactory& factory, const String& value);

        virtual sp<Visibility> build(const sp<Scope>& args);

    private:
        sp<Builder<Boolean>> _visible;
    };

protected:
    sp<BooleanWrapper> _visible;

};

}

#endif
