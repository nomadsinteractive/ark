#ifndef ARK_CORE_IMPL_NUMERIC_STALKER_H_
#define ARK_CORE_IMPL_NUMERIC_STALKER_H_

#include "core/forwarding.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

namespace ark {

class Stalker : public Numeric {
public:
    Stalker(const sp<Numeric>& t, const sp<Numeric>& target, float s0, float eta);

    virtual float val() override;
    virtual bool update(uint64_t timestamp) override;

private:
    void doChase(float s0, float v0, float target, float dt);

private:
    sp<Numeric> _t;
    sp<Numeric> _target;
    float _a;
    float _c;
    float _o;
    float _t0;
    float _c0;
    float _eta;
    float _t_modifier;

    float _target_locked;
};

}

#endif
