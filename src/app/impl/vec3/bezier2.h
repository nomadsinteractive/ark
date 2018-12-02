#ifndef ARK_APP_IMPL_VEC3_BEZIER2_H_
#define ARK_APP_IMPL_VEC3_BEZIER2_H_

#include "core/forwarding.h"
#include "core/inf/builder.h"
#include "core/inf/variable.h"
#include "core/types/safe_ptr.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/base/v3.h"

#include "renderer/forwarding.h"

namespace ark {

class Bezier2 : public Vec3 {
public:
    Bezier2(const sp<Numeric>& t, const sp<Numeric>& v, const sp<Vec2>& p1, const sp<Vec2>& p2, const sp<Vec2>& p3, const sp<Runnable>& onarrival);

    virtual V3 val() override;

//  [[plugin::resource-loader("bezier2")]]
    class BUILDER : public Builder<Vec3> {
    public:
        BUILDER(BeanFactory& parent, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext);

        virtual sp<Vec3> build(const sp<Scope>& args) override;

    private:
        sp<Builder<Numeric>> _t;
        sp<Builder<Numeric>> _v;
        sp<Builder<Vec2>> _p1, _p2, _p3;
        SafePtr<Builder<Runnable>> _on_arrival;
        sp<ResourceLoaderContext> _resource_loader_context;

    };

private:
    V2 interpolate(const V2& p1, const V2& p2, float t);

private:
    sp<Numeric> _t;
    sp<Numeric> _v;
    sp<Vec2> _p1, _p2, _p3;
    sp<Runnable> _on_arrival;

    V2 _last;
    float _last_t;
    float _p;
};

}

#endif
