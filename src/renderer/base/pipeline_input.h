#pragma once

#include <map>
#include <vector>

#include "core/base/api.h"
#include "core/collection/table.h"
#include "core/types/optional.h"
#include "core/types/shared_ptr.h"

#include "graphics/base/camera.h"
#include "graphics/base/render_layer_snapshot.h"

#include "renderer/forwarding.h"
#include "renderer/base/attribute.h"
#include "renderer/base/shader_stage.h"

namespace ark {

class ARK_API PipelineInput {
public:
    struct ARK_API AttributeOffsets {
        AttributeOffsets();
        AttributeOffsets(const PipelineInput& input);

        uint32_t stride() const;

        int32_t _offsets[Attribute::USAGE_COUNT];
        uint32_t _stride;
    };

    class ARK_API StreamLayout {
    public:
        StreamLayout();
        DEFAULT_COPY_AND_ASSIGN(StreamLayout);

        uint32_t stride() const;

        const Table<String, Attribute>& attributes() const;
        void addAttribute(String name, Attribute attribute);

        Optional<const Attribute&> getAttribute(Attribute::Usage layoutType) const;
        Optional<const Attribute&> getAttribute(const String& name) const;
        int32_t getAttributeOffset(Attribute::Usage layoutType) const;
        [[deprecated]]
        int32_t getAttributeOffset(const String& name) const;

        void align(uint32_t alignment);

    private:
        Table<String, Attribute> _attributes;
        uint32_t _stride;
    };

    struct ARK_API UBO {
        UBO(uint32_t binding);

        RenderLayerSnapshot::UBOSnapshot snapshot(const RenderRequest& renderRequest) const;

        uint32_t binding() const;
        size_t size() const;

        const Table<String, sp<Uniform>>& uniforms() const;
        void addUniform(const sp<Uniform>& uniform);

        const std::vector<std::pair<uintptr_t, size_t>>& slots() const;

        void initialize();
        void doSnapshot(uint64_t timestamp, bool force) const;

        uint32_t _binding;
        Table<String, sp<Uniform>> _uniforms;

        std::vector<std::pair<uintptr_t, size_t>> _slots;
        ShaderStageSet _stages;

        bytearray _dirty_flags;
        bytearray _buffer;

        friend class PipelineBuildingContext;
        friend class PipelineInput;
    };

    struct SSBO {
        SSBO() = default;
        SSBO(Buffer buffer, uint32_t binding);
        DEFAULT_COPY_AND_ASSIGN(SSBO);

        Buffer _buffer;
        uint32_t _binding;

        ShaderStageSet _stages;
    };

    struct BindingSet {
        uint32_t _binding = std::numeric_limits<uint32_t>::max();
        ShaderStageSet _stages;

        uint32_t addStage(Enum::ShaderStageBit stage, uint32_t binding);
    };
public:
    PipelineInput(const sp<Camera>& camera);

    void initialize(const PipelineBuildingContext& buildingContext);

    const Camera& camera() const;
    const std::vector<sp<UBO>>& ubos() const;

    std::vector<SSBO>& ssbos();
    const std::vector<SSBO>& ssbos() const;

    sp<RenderLayerSnapshot::BufferObject> takeBufferSnapshot(const RenderRequest& renderRequest, bool isComputeStage) const;

    const std::map<uint32_t, StreamLayout>& streamLayouts() const;
    std::map<uint32_t, StreamLayout>& streamLayouts();

    size_t samplerCount() const;
    const std::vector<String>& samplerNames() const;

    void addAttribute(String name, Attribute attribute);

    const StreamLayout& getStreamLayout(uint32_t divisor) const;

    Optional<const Attribute&> getAttribute(const String& name) const;

    sp<Uniform> getUniform(const String& name) const;

private:
    Camera _camera;

    std::vector<sp<UBO>> _ubos;
    std::vector<SSBO> _ssbos;

    std::map<uint32_t, StreamLayout> _stream_layouts;
    std::vector<String> _sampler_names;

    friend class PipelineBuildingContext;
    friend class PipelineLayout;
    friend class Shader;
};

}
