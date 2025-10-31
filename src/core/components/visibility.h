#pragma once

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/inf/builder.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

namespace ark {

//[[script::bindings::extends(Boolean)]]
class ARK_API Visibility final : public Boolean {
public:
//  [[script::bindings::auto]]
    Visibility(bool visible = false);
//  [[script::bindings::auto]]
    Visibility(const sp<Boolean>& visible);

    bool val() override;
    bool update(uint32_t tick) override;

//  [[script::bindings::auto]]
    void show();
//  [[script::bindings::auto]]
    void hide();
//  [[script::bindings::auto]]
    void set(bool visible);
//  [[script::bindings::auto]]
    void set(const sp<Boolean>& visible);

//  [[plugin::builder::by-value]]
    class DICTIONARY final : public Builder<Visibility> {
    public:
        DICTIONARY(BeanFactory& factory, const String& value);

        sp<Visibility> build(const Scope& args) override;

    private:
        sp<Builder<Boolean>> _visible;
    };

protected:
    sp<BooleanWrapper> _visible;

};

}
