#pragma once

#include "graphics/forwarding.h"
#include "graphics/base/transform.h"

namespace ark {

class TransformNone : public Transform::Delegate {
public:

    void snapshot(const Transform& transform, const V3& postTranslate, Transform::Snapshot& snapshot) const override;
    V3 transform(const Transform::Snapshot& snapshot, const V3& position) const override;
    M4 toMatrix(const Transform::Snapshot& snapshot) const override;

private:
    struct Snapshot {
        V3 postTranslate;
    };

};

}
