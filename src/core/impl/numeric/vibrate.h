#pragma once

#include "core/forwarding.h"
#include "core/inf/builder.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

namespace ark {

class Vibrate : public Numeric {
public:
    Vibrate(const sp<Numeric>& t, float a, float c, float o);

    float val() override;
    bool update(uint64_t timestamp) override;

//  [[plugin::builder("vibrate")]]
    class BUILDER : public Builder<Numeric> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        sp<Numeric> build(const Scope& args) override;

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
