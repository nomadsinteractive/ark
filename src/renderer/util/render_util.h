#pragma once

#include "core/forwarding.h"
#include "core/inf/array.h"

#include "graphics/forwarding.h"

#include "renderer/base/pipeline_layout.h"
#include "renderer/base/shader_preprocessor.h"
#include "renderer/base/texture.h"

namespace ark {

class ARK_API RenderUtil {
public:
    static bytearray makeUnitCubeVertices(bool flipWindingOrder);
    static Attribute makePredefinedAttribute(const String& name, const String& type, Attribute::Usage layoutType = Attribute::USAGE_CUSTOM);

    static uint32_t hash(const element_index_t* buf, size_t len);

    static Attribute::Usage toAttributeLayoutType(const String& name, const String& type);

    static String outAttributeName(const String& name, enums::ShaderStageBit preStage);

    static bool isScissorEnabled(const Rect& scissor);

    static uint32_t getChannelSize(Texture::Format format);
    static uint32_t getPixelSize(Texture::Format format);
    static uint32_t getComponentSize(Texture::Format format);

    static Vector<uint32_t> compileSPIR(StringView source, enums::ShaderStageBit stage, enums::RenderingBackendBit renderTarget, uint32_t targetLanguageVersion = 0);

    static Vector<ShaderPreprocessor::Declaration> setupLayoutLocation(const PipelineBuildingContext& context, const ShaderPreprocessor::DeclarationList& declarations);
    static uint32_t setLayoutDescriptor(const Vector<ShaderPreprocessor::Declaration>& declarations, const String& qualifierName, uint32_t start, int32_t set = -1);
    static uint32_t setLayoutDescriptor(const ShaderPreprocessor::DeclarationList& declarations, const String& descriptor, uint32_t start, int32_t set = -1);
    static uint32_t setLayoutDescriptor(const ShaderPreprocessor::DeclarationList& ins, const ShaderPreprocessor::DeclarationList& outs, const String& qualifierName, uint32_t start);
};

}
