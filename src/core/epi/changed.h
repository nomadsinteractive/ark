#ifndef ARK_CORE_EPI_CHANGED_H_
#define ARK_CORE_EPI_CHANGED_H_

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/inf/builder.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

namespace ark {

class ARK_API Changed {
public:
    Changed(bool changed = true);
    Changed(const sp<Boolean>& delegate);

    bool dirty() const;
    void notify();

    void set(const sp<Boolean>& dirty);

    const sp<Boolean> toBoolean() const;

//  [[plugin::builder]]
    class BUILDER : public Builder<Changed> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        virtual sp<Changed> build(const sp<Scope>& args) override;

    private:
        sp<Builder<Boolean>> _delegate;
    };

//  [[plugin::builder::by-value]]
    class DICTIONARY : public Builder<Changed> {
    public:
        DICTIONARY(BeanFactory& factory, const String& value);

        virtual sp<Changed> build(const sp<Scope>& args);

    private:
        bool _changed;
        sp<Builder<Boolean>> _delegate;
    };


private:
    struct Stub : public Boolean {
        Stub(bool changed);
        Stub(const sp<Boolean>& delegate);

        virtual bool val() override;

        bool _changed;
        sp<Boolean> _delegate;

    };

    sp<Stub> _stub;
};

}

#endif
