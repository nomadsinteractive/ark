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

namespace ark {

class ARK_API PipelineInput {
public:
//TODO: Use Attribute::LayoutType instead
    enum AttributeName {
        ATTRIBUTE_NAME_TEX_COORDINATE,
        ATTRIBUTE_NAME_NORMAL,
        ATTRIBUTE_NAME_TANGENT,
        ATTRIBUTE_NAME_BITANGENT,
        ATTRIBUTE_NAME_BONE_IDS,
        ATTRIBUTE_NAME_BONE_WEIGHTS,
        ATTRIBUTE_NAME_MODEL_MATRIX,
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

        size_t stride() const;

        int32_t _offsets[ATTRIBUTE_NAME_COUNT];
        AttributeName _last_attribute;
    };

    class ARK_API StreamLayout {
    public:
        StreamLayout();
        DEFAULT_COPY_AND_ASSIGN(StreamLayout);

        uint32_t stride() const;

        const Table<String, Attribute>& attributes() const;
        void addAttribute(String name, Attribute attribute);

        Optional<const Attribute&> getAttribute(Attribute::LayoutType layoutType) const;
        Optional<const Attribute&> getAttribute(const String& name) const;
        int32_t getAttributeOffset(Attribute::LayoutType layoutType) const;
        [[deprecated]]
        int32_t getAttributeOffset(const String& name) const;

        void align();

    private:
        Table<String, Attribute> _attributes;
        uint32_t _stride;
    };

    class ARK_API UBO {
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
    PipelineInput(const sp<Camera>& camera);

    void initialize(const PipelineBuildingContext& buildingContext);

    Camera& camera();
    const Camera& camera() const;
    const std::vector<sp<UBO>>& ubos() const;

    std::vector<SSBO>& ssbos();
    const std::vector<SSBO>& ssbos() const;

    const std::map<uint32_t, StreamLayout>& layouts() const;
    std::map<uint32_t, StreamLayout>& layouts();

    size_t samplerCount() const;
    bool hasSampler(const String& name) const;
    const std::vector<String>& samplerNames() const;

    bool hasImage(const String& name) const;
    const std::vector<String>& imageNames() const;

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
    std::vector<String> _image_names;

    friend class PipelineBuildingContext;
    friend class PipelineLayout;
    friend class Shader;
};

}
