#ifndef ARK_RENDERER_BASE_MODEL_H_
#define ARK_RENDERER_BASE_MODEL_H_

#include "core/forwarding.h"
#include "core/inf/array.h"
#include "core/types/shared_ptr.h"

#include "graphics/base/metrics.h"
#include "graphics/base/v3.h"

#include "renderer/forwarding.h"

namespace ark {

class ARK_API Model {
public:
    Model() = default;
    Model(const array<element_index_t>& indices, const sp<Vertices>& vertices, const Metrics& metrics = {V3(1.0f), V3(1.0f), V3(0)});
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(Model);

    const array<element_index_t>& indices() const;
    const sp<Vertices>& vertices() const;

    const Metrics& metrics() const;

    void writeToStream(VertexStream& buf, const V3& size) const;

private:
    V3 toScale(const V3& renderObjectSize) const;

private:
    array<element_index_t> _indices;
    sp<Vertices> _vertices;
    Metrics _metrics;
};

}

#endif
