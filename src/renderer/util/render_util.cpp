#include "renderer/util/render_util.h"

#include "core/types/shared_ptr.h"
#include "core/util/input_type.h"

#include "graphics/base/rect.h"
#include "graphics/base/v3.h"

#include "renderer/base/model.h"
#include "renderer/impl/vertices/vertices_nine_patch_quads.h"
#include "renderer/impl/vertices/vertices_nine_patch_triangle_strips.h"
#include "renderer/impl/vertices/vertices_point.h"
#include "renderer/impl/vertices/vertices_quad.h"


namespace ark {

bytearray RenderUtil::makeUnitCubeVertices(bool flipWindingOrder)
{
    static float vertices[] = {
        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,

        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f
    };

    if(flipWindingOrder)
    {
        sp<ByteArray::Fixed<sizeof(vertices)>> flipped = sp<ByteArray::Fixed<sizeof(vertices)>>::make();
        memcpy(flipped->buf(), vertices, flipped->size());
        V3* buf = reinterpret_cast<V3*>(flipped->buf());
        for(uint32_t i = 0; i < 6; ++i, buf += 4)
            std::swap(buf[1], buf[2]);
        return flipped;
    }

    return sp<ByteArray::Borrowed>::make(reinterpret_cast<uint8_t*>(vertices), sizeof(vertices));
}

Attribute RenderUtil::makePredefinedAttribute(const String& name, const String& type)
{
    if(type == "vec3" || type == "v3f")
        return Attribute(name, Attribute::TYPE_FLOAT, type, 3, false);
    if(type == "vec2" || type == "v2f")
        return Attribute(name, Attribute::TYPE_FLOAT, type, 2, false);
    if(type == "float")
        return Attribute(name, Attribute::TYPE_FLOAT, type, 1, false);
    if(type == "int")
        return Attribute(name, Attribute::TYPE_INTEGER, type, 1, false);
    if(type == "vec4" || type == "v4f")
        return Attribute(name, Attribute::TYPE_FLOAT, type, 4, false);
    if(type == "vec4b" || type == "v4b")
        return Attribute(name, Attribute::TYPE_UBYTE, type, 4, true);
    if(type == "vec3b" || type == "v3fb")
        return Attribute(name, Attribute::TYPE_UBYTE, type, 3, true);
    if(type == "uint8")
        return Attribute(name, Attribute::TYPE_UBYTE, type, 1, false);
    if(type == "mat4")
        return Attribute(name, Attribute::TYPE_FLOAT, type, 16, false);
    if(type == "ivec4")
        return Attribute(name, Attribute::TYPE_INTEGER, type, 4, false);
    DFATAL("Unknown attribute type \"%s\"", type.c_str());
    return Attribute();
}

Model RenderUtil::makeUnitQuadModel()
{
    return Model(InputType::makeElementIndexInput(std::initializer_list<element_index_t>({0, 2, 1, 2, 3, 1})), sp<VerticesQuad>::make(), sp<Metrics>::make(Metrics::unit()));
}

Model RenderUtil::makeUnitNinePatchTriangleStripsModel()
{
    return Model(InputType::makeElementIndexInput(std::initializer_list<element_index_t>({0, 4, 1, 5, 2, 6, 3, 7, 7, 4, 4, 8, 5, 9, 6, 10, 7, 11, 11, 8, 8, 12, 9, 13, 10, 14, 11, 15})), sp<VerticesNinePatchTriangleStrips>::make(), sp<Metrics>::make(Metrics::unit()));
}

Model RenderUtil::makeUnitNinePatchQuadsModel()
{
    return Model(InputType::makeElementIndexInput(std::initializer_list<element_index_t>({0, 2, 1, 2, 3, 1, 6, 8, 7, 8, 9, 7, 12, 14, 13, 14, 15, 13, 18, 20, 19, 20, 21, 19, 24, 26, 25, 26, 27, 25, 30, 32, 31, 32, 33, 31, 36, 38, 37, 38, 39, 37, 42, 44, 43, 44, 45, 43, 48, 50, 49, 50, 51, 49})), sp<VerticesNinePatchQuads>::make(), sp<Metrics>::make(Metrics::unit()));
}

Model RenderUtil::makeUnitPointModel()
{
    return Model(InputType::makeElementIndexInput(std::initializer_list<element_index_t>({0})), sp<VerticesPoint>::make(), sp<Metrics>::make(V3(0), V3(0)));
}

uint32_t RenderUtil::hash(const element_index_t* buf, size_t len)
{
    uint32_t h = 0;
    for(size_t i = 0; i < len; ++i)
        h = h * 101 + buf[i];
    return h;
}

String RenderUtil::outAttributeName(const String& name, PipelineInput::ShaderStage preStage)
{
    DCHECK(preStage == PipelineInput::SHADER_STAGE_NONE || preStage == PipelineInput::SHADER_STAGE_VERTEX, "Only none and vertex stage's out attribute name supported");
    const char sPrefix[][8] = {"a_", "v_"};
    const String prefix = sPrefix[preStage + 1];
    return name.startsWith(prefix) ? name : prefix + Strings::capitalizeFirst(name);
}

uint32_t RenderUtil::getComponentSize(Texture::Format format)
{
    if(format == Texture::FORMAT_AUTO)
        return 4;

    uint32_t unitSize = 1;
    if(format & Texture::FORMAT_F16 || (format & Texture::FORMAT_I16) == Texture::FORMAT_I16)
        unitSize = 2;
    else if(format & Texture::FORMAT_F32 || (format & Texture::FORMAT_I32) == Texture::FORMAT_I32)
        unitSize = 4;

    return (static_cast<uint32_t>(format & Texture::FORMAT_RGBA) + 1) * unitSize;
}

}
