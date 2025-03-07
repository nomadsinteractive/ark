#pragma once

#include "core/base/api.h"
#include "core/base/bit_set_types.h"
#include "core/collection/table.h"
#include "core/types/optional.h"
#include "core/types/shared_ptr.h"

#include "graphics/base/camera.h"
#include "graphics/base/render_layer_snapshot.h"

#include "renderer/forwarding.h"
#include "renderer/base/attribute.h"

namespace ark {

class ARK_API PipelineLayout {
public:
    struct VertexDescriptor {
        VertexDescriptor();
        VertexDescriptor(const PipelineLayout& pipelineLayout);

        void initialize(const PipelineLayout& pipelineLayout);

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

    struct Binding {
        int32_t _location = -1;
        int32_t _set = -1;
    };

    struct ARK_API UBO {
        UBO(uint32_t binding);

        RenderLayerSnapshot::UBOSnapshot snapshot(const RenderRequest& renderRequest) const;

        uint32_t binding() const;
        size_t size() const;

        const Table<String, sp<Uniform>>& uniforms() const;
        void addUniform(const sp<Uniform>& uniform);

        const Vector<std::pair<uintptr_t, size_t>>& slots() const;

        void initialize();
        void doSnapshot(uint64_t timestamp, bool force) const;

        uint32_t _binding;
        Table<String, sp<Uniform>> _uniforms;

        Vector<std::pair<uintptr_t, size_t>> _slots;
        ShaderStageSet _stages;

        bytearray _dirty_flags;
        bytearray _buffer;

        friend class PipelineBuildingContext;
        friend class PipelineLayout;
    };

    struct SSBO {
        SSBO() = default;
        SSBO(Buffer buffer, Binding binding);
        DEFAULT_COPY_AND_ASSIGN(SSBO);

        Buffer _buffer;
        Binding _binding;

        ShaderStageSet _stages;
    };

    struct DescriptorSet {
        ShaderStageSet _stages;
        Binding _binding;

        uint32_t addStage(Enum::ShaderStageBit stage, uint32_t binding);
    };

public:
    PipelineLayout();

    void initialize(const PipelineBuildingContext& buildingContext);

    const Vector<sp<UBO>>& ubos() const;
    const Vector<SSBO>& ssbos() const;

    sp<RenderLayerSnapshot::BufferObject> takeBufferSnapshot(const RenderRequest& renderRequest, bool isComputeStage) const;

    const Map<uint32_t, StreamLayout>& streamLayouts() const;
    void setStreamLayoutAlignment(uint32_t alignment);

    const VertexDescriptor& vertexDescriptor() const;

    const Table<String, DescriptorSet>& samplers() const;
    const Table<String, DescriptorSet>& images() const;

    uint32_t colorAttachmentCount() const;

    void addAttribute(String name, Attribute attribute);
    Optional<const Attribute&> getAttribute(const String& name) const;
    sp<Uniform> getUniform(const String& name) const;

    const StreamLayout& getStreamLayout(uint32_t divisor) const;

private:
    Vector<sp<UBO>> _ubos;
    Vector<SSBO> _ssbos;

    Map<uint32_t, StreamLayout> _stream_layout;
    VertexDescriptor _vertex_descriptor;

    Table<String, DescriptorSet> _samplers;
    Table<String, DescriptorSet> _images;

    uint32_t _color_attachment_count;

    friend class PipelineBuildingContext;
    friend class PipelineDescriptor;
    friend class Shader;
};

}
