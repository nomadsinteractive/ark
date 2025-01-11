#pragma once

#include "renderer/base/atlas.h"
#include "renderer/inf/vertices.h"

namespace ark {

class VerticesPoint final : public Vertices {
public:
    VerticesPoint();
    VerticesPoint(const Atlas::Item& atlasItem);

    void write(VertexWriter& buf, const V3& size) override;

private:
    Atlas::Item _atlas_item;
};

}
