#ifndef ARK_RENDERER_BASE_PIPELINE_INPUT_H_
#define ARK_RENDERER_BASE_PIPELINE_INPUT_H_

#include <map>
#include <vector>

#include "core/base/api.h"
#include "core/collection/table.h"
#include "core/types/shared_ptr.h"

#include "graphics/base/layer.h"

#include "renderer/forwarding.h"
#include "renderer/base/attribute.h"
#include "renderer/base/shader.h"

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

    class UBO {
    public:
        UBO(uint32_t binding);

        Layer::UBOSnapshot snapshot(MemoryPool& memoryPool) const;

        void notify() const;

        uint32_t binding() const;
        size_t size() const;

        const std::vector<sp<Uniform>>& uniforms() const;
        const std::vector<std::pair<uintptr_t, size_t>>& slots() const;

        void addStage(Shader::Stage stage);
        const std::set<Shader::Stage>& stages() const;

    private:
        void initialize();
        void addUniform(const sp<Uniform>& uniform);
        void doSnapshot(bool force) const;

    private:
        std::vector<sp<Uniform>> _uniforms;
        uint32_t _binding;

        std::vector<std::pair<uintptr_t, size_t>> _slots;
        std::set<Shader::Stage> _stages;

        bytearray _dirty_flags;
        bytearray _buffer;

        friend class PipelineInput;
    };

public:
    PipelineInput();

    void initialize(const PipelineBuildingContext& buildingContext);

    const std::vector<sp<UBO>>& ubos() const;

    const std::map<uint32_t, Stream>& streams() const;
    std::map<uint32_t, Stream>& streams();

    uint32_t samplerCount() const;

    void addAttribute(String name, Attribute attribute);

    std::vector<std::pair<uint32_t, Buffer>> makeInstancedArrays(const RenderController& renderController) const;

    const PipelineInput::Stream& getStream(uint32_t divisor) const;

    const Attribute& getAttribute(const String& name, uint32_t divisor = 0) const;
    int32_t getAttributeOffset(const String& name, uint32_t divisor = 0) const;

private:
    std::vector<sp<UBO>> _ubos;
    std::map<uint32_t, Stream> _streams;
    uint32_t _sampler_count;

    friend class PipelineLayout;
};

}

#endif
