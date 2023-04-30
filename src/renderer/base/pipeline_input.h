#pragma once

#include <map>
#include <vector>

#include "core/base/api.h"
#include "core/collection/table.h"
#include "core/types/optional.h"
#include "core/types/shared_ptr.h"

#include "graphics/base/render_layer_snapshot.h"

#include "renderer/forwarding.h"
#include "renderer/base/attribute.h"

namespace ark {

class ARK_API PipelineInput {
public:
    enum AttributeName {
        ATTRIBUTE_NAME_TEX_COORDINATE,
        ATTRIBUTE_NAME_NORMAL,
        ATTRIBUTE_NAME_TANGENT,
        ATTRIBUTE_NAME_BITANGENT,
        ATTRIBUTE_NAME_BONE_IDS,
        ATTRIBUTE_NAME_BONE_WEIGHTS,
        ATTRIBUTE_NAME_NODE_ID,
        ATTRIBUTE_NAME_MATERIAL_ID,
        ATTRIBUTE_NAME_COUNT
    };

    enum ShaderStage {
        SHADER_STAGE_NONE = -1,
        SHADER_STAGE_VERTEX,
#ifndef ANDROID
        SHADER_STAGE_TESSELLATION_CTRL,
        SHADER_STAGE_TESSELLATION_EVAL,
        SHADER_STAGE_GEOMETRY,
#endif
        SHADER_STAGE_FRAGMENT,
        SHADER_STAGE_COMPUTE,
        SHADER_STAGE_COUNT
    };

    struct ARK_API AttributeOffsets {
        AttributeOffsets();
        AttributeOffsets(const PipelineInput& input);

        int32_t _offsets[ATTRIBUTE_NAME_COUNT];
    };

    class Stream {
    public:
        Stream();
        DEFAULT_COPY_AND_ASSIGN(Stream);

        uint32_t stride() const;

        const Table<String, Attribute>& attributes() const;
        void addAttribute(String name, Attribute attribute);

        Optional<const Attribute&> getAttribute(const String& name) const;
        int32_t getAttributeOffset(const String& name) const;

        void align();

    private:
        uint32_t _stride;

        Table<String, Attribute> _attributes;
    };

    class UBO {
    public:
        UBO(uint32_t binding);

        RenderLayerSnapshot::UBOSnapshot snapshot(const RenderRequest& renderRequest) const;

        uint32_t binding() const;
        size_t size() const;

        const Table<String, sp<Uniform>>& uniforms() const;
        const std::vector<std::pair<uintptr_t, size_t>>& slots() const;

        void addStage(ShaderStage stage);
        const std::set<ShaderStage>& stages() const;

    private:
        void initialize();
        void addUniform(const sp<Uniform>& uniform);
        void doSnapshot(uint64_t timestamp, bool force) const;

    private:
        Table<String, sp<Uniform>> _uniforms;
        uint32_t _binding;

        std::vector<std::pair<uintptr_t, size_t>> _slots;
        std::set<ShaderStage> _stages;

        bytearray _dirty_flags;
        bytearray _buffer;

        friend class PipelineInput;
    };

    struct SSBO {
        SSBO() = default;
        SSBO(Buffer buffer, uint32_t binding);
        DEFAULT_COPY_AND_ASSIGN(SSBO);

        Buffer _buffer;
        uint32_t _binding;

        std::set<ShaderStage> _stages;

    };

public:
    PipelineInput();

    void initialize(const PipelineBuildingContext& buildingContext);

    const std::vector<sp<UBO>>& ubos() const;

    std::vector<SSBO>& ssbos();
    const std::vector<SSBO>& ssbos() const;

    const std::map<uint32_t, Stream>& streams() const;
    std::map<uint32_t, Stream>& streams();

    size_t samplerCount() const;
    const std::vector<String>& samplerNames() const;

    const std::vector<String>& imageNames() const;

    void addAttribute(String name, Attribute attribute);

    const PipelineInput::Stream& getStream(uint32_t divisor) const;

    Optional<const Attribute&> getAttribute(const String& name) const;

    sp<Uniform> getUniform(const String& name) const;

private:
    std::vector<sp<UBO>> _ubos;
    std::vector<SSBO> _ssbos;

    std::map<uint32_t, Stream> _streams;
    std::vector<String> _sampler_names;
    std::vector<String> _image_names;

    friend class PipelineLayout;
};

}
