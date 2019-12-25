#ifndef ARK_CORE_IMPL_NUMERIC_DAMPER_H_
#define ARK_CORE_IMPL_NUMERIC_DAMPER_H_

#include "core/forwarding.h"
#include "core/inf/builder.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

namespace ark {

class Damper : public Numeric {
public:
    Damper(const sp<Numeric>& t, float a, float c, float o);

    virtual float val() override;
    virtual bool update(uint64_t timestamp) override;

//  [[plugin::builder("damper")]]
    class BUILDER : public Builder<Numeric> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        virtual sp<Numeric> build(const Scope& args) override;

    private:
        float v2c(float v, float a) const;

    private:
        sp<Builder<Duration>> _duration;
        sp<Builder<Numeric>> _v;
        sp<Builder<Numeric>> _s1, _s2;
    };

private:
    sp<Numeric> _t;
    float _a, _c, _o;

};

}

#endif
