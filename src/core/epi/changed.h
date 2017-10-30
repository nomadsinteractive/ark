#ifndef ARK_CORE_EPI_CHANGED_H_
#define ARK_CORE_EPI_CHANGED_H_

#include <atomic>


#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"

namespace ark {

class ARK_API Changed {
public:
    Changed(bool changed = true);
    Changed(const sp<Boolean>& delegate);

    bool hasChanged();
    void change();

//  [[plugin::builder]]
    class BUILDER : public Builder<Changed> {
    public:
        BUILDER(BeanFactory& parent, const document& doc);

        virtual sp<Changed> build(const sp<Scope>& args) override;

    private:
        sp<Builder<Boolean>> _delegate;
    };

//  [[plugin::builder::by-value]]
    class DICTIONARY : public Builder<Changed> {
    public:
        DICTIONARY(BeanFactory& parent, const String& value);

        virtual sp<Changed> build(const sp<Scope>& args);

    private:
        bool _changed;
        sp<Builder<Boolean>> _delegate;
    };


protected:
    bool _changed;
    sp<Boolean> _delegate;

};

}

#endif
