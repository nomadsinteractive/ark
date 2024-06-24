#include "plugin/gltf/impl/model_loader/model_loader_gltf.h"

#include <vector>

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define TINYGLTF_NOEXCEPTION
#define JSON_NOEXCEPTION
#include <tiny_gltf.h>

#include "core/base/string.h"
#include "core/inf/readable.h"

#include "graphics/base/mat.h"
#include "graphics/base/material.h"
#include "graphics/util/matrix_util.h"

#include "renderer/base/atlas.h"
#include "renderer/base/material_bundle.h"
#include "renderer/base/render_controller.h"
#include "renderer/base/render_engine.h"
#include "renderer/base/shader_data_type.h"

namespace ark::plugin::gltf {

namespace {

template<typename T> struct SBufferReadData {
    const uint8_t* SrcData = nullptr;
    size_t SrcStride = 0;

    std::vector<T> DstData;
    size_t DstStride = 0;

    ShaderDataType _shader_data_type;
    size_t NumElements = 0;
};

template<typename T, typename TReadElementType> void readBufferData(SBufferReadData<T>& bufferReadData) {
    if constexpr(!std::is_same_v<TReadElementType, void>)
        ASSERT(sizeof(TReadElementType) == bufferReadData._shader_data_type.size());

    size_t SrcOffset = 0;
    size_t DstOffset = 0;
    size_t dstSize = bufferReadData._shader_data_type.size();
    uint8_t* dstDataPtr = reinterpret_cast<uint8_t*>(bufferReadData.DstData.data());

    for (size_t i = 0; i < bufferReadData.NumElements; ++i)
    {
        memcpy(dstDataPtr + DstOffset, bufferReadData.SrcData + SrcOffset, dstSize);

        SrcOffset += bufferReadData.SrcStride;
        DstOffset += bufferReadData.DstStride;
    }
}

ShaderDataType getShaderDataTypeFromAccessor(const tinygltf::Accessor& accessor)
{
    uint32_t numOfComponents = 0;

    switch (accessor.type) {
    case TINYGLTF_TYPE_SCALAR:
        numOfComponents = 1;
        break;
    case TINYGLTF_TYPE_VEC2:
        numOfComponents = 2;
        break;
    case TINYGLTF_TYPE_VEC3:
        numOfComponents = 3;
        break;
    case TINYGLTF_TYPE_VEC4:
        numOfComponents = 4;
        break;
    default:
        DFATAL("Unsupported data type in gltf model!");
    }

    ShaderDataType::ComponentType componentType = ShaderDataType::TYPE_NONE;
    switch (accessor.componentType)
    {
    case TINYGLTF_COMPONENT_TYPE_BYTE:
        componentType = ShaderDataType::TYPE_BYTE;
        break;
    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
        componentType = ShaderDataType::TYPE_UBYTE;
        break;
    case TINYGLTF_COMPONENT_TYPE_SHORT:
        componentType = ShaderDataType::TYPE_SHORT;
        break;
    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
        componentType = ShaderDataType::TYPE_USHORT;
        break;
    case TINYGLTF_COMPONENT_TYPE_INT:
        componentType = ShaderDataType::TYPE_INTEGER;
        break;
    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
        componentType = ShaderDataType::TYPE_UINTEGER;
        break;
    case TINYGLTF_COMPONENT_TYPE_FLOAT:
        componentType = ShaderDataType::TYPE_FLOAT;
        break;
    default:
        FATAL("Unsupported data component type in gltf model!");
    }

    return {componentType, numOfComponents};
}

std::vector<sp<Material>> loadMaterials(tinygltf::Model const& gltfModel, MaterialBundle& materialBundle) {
    std::vector<sp<Material>> materials(gltfModel.materials.size());

    for(size_t i = 0; i < gltfModel.materials.size(); ++i) {
        const tinygltf::Material& gltfMaterial = gltfModel.materials.at(i);
        String mName = gltfMaterial.name;
        sp<Material>& material = materials[i];
        if(!(material = materialBundle.getMaterial(mName))) {
            material = sp<Material>::make(i, std::move(mName));
            // Albedo
            if(gltfMaterial.pbrMetallicRoughness.baseColorTexture.index == -1) {
                const std::vector<double>& vertexColorData = gltfMaterial.pbrMetallicRoughness.baseColorFactor;

                V4 vertexColor(static_cast<float>(vertexColorData.at(0)), static_cast<float>(vertexColorData.at(1)),
                               static_cast<float>(vertexColorData.at(2)), static_cast<float>(vertexColorData.at(3)));
                material->baseColor()->setColor(sp<Vec4::Const>::make(vertexColor));
            }

            // TODO: Normals
            // TODO: MetallicRoughness

        }
    }

    return materials;
}

template<typename T> SBufferReadData<T> getBufferReadData(const tinygltf::Accessor* accessor, const tinygltf::BufferView& BufferView, const tinygltf::Buffer& Buffer,
                                                          const ShaderDataType& shaderDataType, size_t componentSize) {
    // clang-format off
    ///////////////////////////////////////////////////////////////////////
    // [--Buffer Data-----------------------------------------------------]
    // ---BV Offset-[--BufferView------------------------------------]----
    // --------------------AccessorOffset-[--------------------------]----
    // -----------------------------------[---Stride---]------------------
    // -----------------------------------[-Data1-]-----[-Data2-]---------
    // -----------------------------------[x--y--z]-----[x--y--z]---------
    ///////////////////////////////////////////////////////////////////////
    // clang-format on

    uint32_t ElementSize = shaderDataType.size();

    // BufferView
    size_t BufferViewOffset = BufferView.byteOffset;
    size_t BufferViewLength = BufferView.byteLength;
    size_t BufferViewStride = BufferView.byteStride; // 0 for tightly packed

    // Accessor
    size_t AccessorOffset = 0;
    size_t NumElements = 0;
    if (accessor)
    {
        AccessorOffset = accessor->byteOffset;
        NumElements = accessor->count;
    }
    else
    {
        NumElements = BufferViewLength / ElementSize;
    }

    size_t SrcElementSize = ElementSize;
    size_t SrcStride = BufferViewStride == 0 ? SrcElementSize : BufferViewStride;
    size_t SrcDataStartOffset = BufferViewOffset + AccessorOffset;

    size_t DstElementSize = shaderDataType.numberOfComponent() * componentSize;
    size_t DstStride = DstElementSize;
    size_t DstDataBytes = NumElements * DstElementSize;

    ASSERT(DstDataBytes % sizeof(T) == 0);

    SBufferReadData<T> bufferReadData;
    bufferReadData.SrcData = Buffer.data.data() + SrcDataStartOffset;
    bufferReadData.SrcStride = SrcStride;
    bufferReadData.DstData.resize(DstDataBytes / sizeof(T));
    bufferReadData.DstStride = DstStride;
    bufferReadData._shader_data_type = shaderDataType;
    bufferReadData.NumElements = NumElements;

    return bufferReadData;
}

template<typename T, typename ComponentType = void> SBufferReadData<T> getAttributeData(const tinygltf::Model& model, const M4& transform, std::string_view attributeKey, uint32_t attributeValue) {
    const tinygltf::Accessor& accessor = model.accessors.at(attributeValue);
    const tinygltf::BufferView& bufferView = model.bufferViews.at(accessor.bufferView);
    const tinygltf::Buffer& buffer = model.buffers.at(bufferView.buffer);
    const ShaderDataType shaderDataType = getShaderDataTypeFromAccessor(accessor);

    size_t componentSize = 4;
    if constexpr(!std::is_same_v<ComponentType, void>)
        componentSize = sizeof(ComponentType);

    SBufferReadData<T> bufferReadParams = getBufferReadData<T>(&accessor, bufferView, buffer, shaderDataType, componentSize);

    readBufferData<T, ComponentType>(bufferReadParams);
    //TransformAttributeData(
    //    reinterpret_cast<FVector3*>(BufferReadParams.DstData),
    //    BufferReadParams.NumElements,
    //    AttributeKey,
    //    TransformMatrix
    //);

    return bufferReadParams;
}

template<typename T> sp<Array<T>> toArray(std::vector<T> vector) {
    return vector.size() ? sp<typename Array<T>::Vector>::make(std::move(vector)) : nullptr;
}

Mesh processPrimitive(const tinygltf::Model& gltfModel, const std::vector<sp<Material>>& materials, const tinygltf::Primitive& primitive, const M4& TransformMatrix, uint32_t id, String name) {
    std::vector<V3> vertices;
    std::vector<V3> normals;
    std::vector<Mesh::UV> uvs;

    for(const auto& [attributeKey, attributeValue] : primitive.attributes)
    {
        if(attributeKey == "POSITION") {
            SBufferReadData bufferReadData = getAttributeData<V3>(gltfModel, TransformMatrix, attributeKey, attributeValue);
            vertices = std::move(bufferReadData.DstData);
        }
        else if(attributeKey == "NORMAL") {
            SBufferReadData bufferReadData = getAttributeData<V3>(gltfModel, TransformMatrix, attributeKey, attributeValue);
            normals = std::move(bufferReadData.DstData);
        }
        else if(attributeKey == "TEXCOORD_0") {
            SBufferReadData bufferReadData = getAttributeData<V2>(gltfModel, TransformMatrix, attributeKey, attributeValue);
            uvs.reserve(bufferReadData.DstData.size());
            for(const V2& i : bufferReadData.DstData)
                uvs.emplace_back(Atlas::unnormalize(i.x()), Atlas::unnormalize(i.y()));
        }
        else {
            WARN("Ignoring primitive attribute \"%s\"", attributeKey.c_str());
        }
    }

    SBufferReadData bufferReadData = getAttributeData<element_index_t, element_index_t>(gltfModel, TransformMatrix, "", primitive.indices);
    std::vector<element_index_t> indices = std::move(bufferReadData.DstData);
    if(Ark::instance().renderController()->renderEngine()->isViewportFlipped())
    {
        const element_index_t indexSize = indices.size();
        ASSERT(indexSize % 3 == 0);
        for(size_t i = 0; i < indexSize; i += 3)
            std::swap(indices[i + 1], indices[i + 2]);
    }

    ASSERT(primitive.material == -1 || primitive.material < materials.size());
    sp<Material> material = primitive.material >= 0 ? materials.at(primitive.material) : nullptr;

    return Mesh(id, std::move(name), std::move(indices), std::move(vertices), toArray<Mesh::UV>(std::move(uvs)), toArray<V3>(std::move(normals)), nullptr, nullptr, std::move(material));
}

M4 getNodeLocalTransformMatrix(const tinygltf::Node& node)
{
    V3 translation(0);
    if(!node.translation.empty())
        translation = V3(static_cast<float>(node.translation.at(0)), static_cast<float>(node.translation.at(1)), static_cast<float>(node.translation.at(2)));

    V3 scale(1.0f);
    if(!node.scale.empty())
        scale = V3(static_cast<float>(node.scale.at(0)), static_cast<float>(node.scale.at(1)), static_cast<float>(node.scale.at(2)));

    V4 quaternion(0, 0, 0, 1.0f);
    if(!node.rotation.empty())
        quaternion = V4(static_cast<float>(node.rotation.at(0)), static_cast<float>(node.rotation.at(1)), static_cast<float>(node.rotation.at(2)), static_cast<float>(node.rotation.at(3)));

    const M4 translateMatrix = MatrixUtil::translate(M4::identity(), translation);
    const M4 rotateMatrix = MatrixUtil::rotate(M4::identity(), quaternion);
    const M4 scaleMatrix = MatrixUtil::scale(M4::identity(), scale);
    return translateMatrix * rotateMatrix * scaleMatrix;
}

sp<Node> loadNodeHierarchy(const tinygltf::Model& gltfModel, const tinygltf::Node& node, const std::vector<sp<Mesh>>& meshes)
{
    sp<Node> n = sp<Node>::make(node.name, getNodeLocalTransformMatrix(node));

    if(node.mesh != -1)
    {
        ASSERT(node.mesh < meshes.size());
        n->meshes().push_back(meshes.at(node.mesh));
    }

    for(const int32_t i : node.children)
        n->childNodes().push_back(loadNodeHierarchy(gltfModel, gltfModel.nodes.at(i), meshes));
    return n;
}

}

Model ModelImporterGltf::import(const Manifest& manifest, MaterialBundle& materialBundle)
{
    tinygltf::TinyGLTF loader;
    tinygltf::Model gltfModel;
    std::string errString, warnString;
    sp<Readable> readable = Ark::instance().openAsset(manifest.src());

    std::vector<uint8_t> buf(readable->remaining());
    readable->read(buf.data(), static_cast<uint32_t>(buf.size()));
    if(manifest.src().endsWith(".gltf"))
        loader.LoadASCIIFromString(&gltfModel, &errString, &warnString, reinterpret_cast<const char*>(buf.data()), buf.size(), "");
    else
        loader.LoadBinaryFromMemory(&gltfModel, &errString, &warnString, buf.data(), buf.size());

    CHECK(errString.size() == 0, "Loading \"%s\" failed with error: %s", manifest.src().c_str(), errString.c_str());
    CHECK_WARN(warnString.size() == 0, "Warning: \"%s\" %s", manifest.src().c_str(), warnString.c_str());

    CHECK(gltfModel.scenes.size() == 1, "\"%s\" must have only one scene, but this file has %d", manifest.src().c_str(), gltfModel.scenes.size());

    std::vector<sp<Material>> materials = loadMaterials(gltfModel, materialBundle);

    uint32_t meshId = 0, primitiveId = 0;
    std::vector<sp<Mesh>> meshes;
    std::vector<std::vector<uint32_t>> primitives;

    for(const tinygltf::Mesh& i : gltfModel.meshes) {
        uint32_t primitiveBase = 0;
        std::vector<uint32_t> primitiveIds;
        for(const tinygltf::Primitive& j : i.primitives) {
            String primitiveName = i.primitives.size() == 1 ? String(i.name) : Strings::sprintf("%s-%d", i.name.c_str(), primitiveBase++);
            primitiveIds.push_back(primitiveId);
            meshes.push_back(sp<Mesh>::make(processPrimitive(gltfModel, materials, j, M4(), primitiveId++, std::move(primitiveName))));
        }
    }

    const tinygltf::Scene& scene = gltfModel.scenes.at(0);
    sp<Node> rootNode = sp<Node>::make(scene.name, M4::identity());

    for(int32_t i : scene.nodes)
        rootNode->childNodes().push_back(loadNodeHierarchy(gltfModel, gltfModel.nodes.at(i), meshes));

    float aabbMinX(std::numeric_limits<float>::max()), aabbMinY(std::numeric_limits<float>::max()), aabbMinZ(std::numeric_limits<float>::max());
    float aabbMaxX(std::numeric_limits<float>::min()), aabbMaxY(std::numeric_limits<float>::min()), aabbMaxZ(std::numeric_limits<float>::min());
    for(const sp<Mesh>& i : meshes)
        for(const V3& j : i->vertices())
        {
            if(aabbMinX > j.x()) aabbMinX = j.x();
            if(aabbMinY > j.y()) aabbMinY = j.y();
            if(aabbMinZ > j.z()) aabbMinZ = j.z();

            if(aabbMaxX < j.x()) aabbMaxX = j.x();
            if(aabbMaxY < j.y()) aabbMaxY = j.y();
            if(aabbMaxZ < j.z()) aabbMaxZ = j.z();
        }

    return Model(std::move(materials), std::move(meshes), std::move(rootNode), sp<Boundaries>::make(V3(aabbMinX, aabbMinY, aabbMinZ), V3(aabbMaxX, aabbMaxY, aabbMaxZ)));
}

sp<ModelLoader::Importer> ModelImporterGltf::BUILDER::build(const Scope& args)
{
    return sp<ModelImporterGltf>::make();
}

}
