#ifndef ARK_GRAPHICS_INF_VARYING_H_
#define ARK_GRAPHICS_INF_VARYING_H_

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/inf/array.h"

#include "graphics/forwarding.h"

#include "renderer/base/pipeline_input.h"

namespace ark {

class ARK_API Varying {
public:
    virtual ~Varying() = default;

    virtual void bind(const PipelineInput& pipelineInput) = 0;
    virtual ByteArray::Borrowed snapshot(const RenderRequest& renderRequest) = 0;
    virtual void apply(Writable& writer, const ByteArray::Borrowed& snapshot, uint32_t vertexId) = 0;

};

}

#endif
