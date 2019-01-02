#ifndef ARK_RENDERER_BASE_PIPELINE_INPUT_H_
#define ARK_RENDERER_BASE_PIPELINE_INPUT_H_

#include <map>
#include <vector>

#include "core/base/api.h"
#include "core/collection/table.h"
#include "core/types/shared_ptr.h"

#include "renderer/forwarding.h"
#include "renderer/base/attribute.h"

namespace ark {

class PipelineInput {
public:
    class Stream {
    public:
        Stream();
        Stream(uint32_t divisor);
        DEFAULT_COPY_AND_ASSIGN(Stream);

        uint32_t stride() const;

        const Table<String, Attribute>& attributes() const;
        void addAttribute(String name, Attribute attribute);

        const Attribute& getAttribute(const String& name) const;
        int32_t getAttributeOffset(const String& name) const;

        void align();

    private:
        uint32_t _divisor;
        uint32_t _stride;

        Table<String, Attribute> _attributes;
    };

public:
    PipelineInput();

    void initialize(const PipelineBuildingContext& buildingContext);

    const std::vector<sp<Uniform>>& uniforms() const;
    const sp<UBO>& ubo() const;

    const std::map<uint32_t, Stream>& streams() const;
    std::map<uint32_t, Stream>& streams();

    uint32_t samplerCount() const;

    void addAttribute(String name, Attribute attribute);

    std::vector<std::pair<uint32_t, Buffer>> makeInstancedArrays(RenderController& renderController) const;

    const PipelineInput::Stream& getStream(uint32_t divisor) const;

    const Attribute& getAttribute(const String& name, uint32_t divisor = 0) const;
    int32_t getAttributeOffset(const String& name, uint32_t divisor = 0) const;

private:
    sp<UBO> _ubo;
    std::map<uint32_t, Stream> _streams;
    uint32_t _sampler_count;

    friend class PipelineLayout;
};

}

#endif
