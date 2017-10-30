#include "graphics/impl/vv2/vv2_with_transform.h"

#include "graphics/base/transform.h"
#include "graphics/base/v2.h"

namespace ark {

VV2WithTransform::VV2WithTransform(const sp<VV2>& delegate, const sp<VV2>& org, const sp<Transform>& transform)
    : _delegate(delegate), _org(org), _transform(transform)
{
}

V2 VV2WithTransform::val()
{
    const V2 p = _delegate->val();
    const V2 org = _org->val();
    float x, y;
    _transform->snapshot().map(p.x() - org.x(), p.y() - org.y(), org.x(), org.y(), x, y);
    return V2(x, y);
}

}
