#include "gltf/gltf_importer.h"

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define TINYGLTF_NOEXCEPTION
#define JSON_NOEXCEPTION
#include <tiny_gltf.h>

#include "core/base/constants.h"
#include "core/inf/readable.h"
#include "core/util/math.h"
#include "core/util/strings.h"

#include "graphics/base/v4.h"
#include "impl/animation/animation_gltf.h"

#include "renderer/base/animation_uploader.h"
#include "renderer/base/atlas.h"
#include "renderer/base/material_bundle.h"
#include "renderer/base/mesh.h"
#include "renderer/base/model.h"
#include "renderer/base/node.h"
#include "renderer/base/shader_data_type.h"

namespace ark::plugin::gltf {

namespace {

struct AnimationChannel {
	enum PathType { TRANSLATION, ROTATION, SCALE };
	PathType path;
	uint32_t node_id;
	uint32_t samplerIndex;

	V3 _intermediate_translation{};
	V3 _intermediate_scale{};
	V4 _intermediate_quaternion{};
};

struct AnimationSampler {
	enum InterpolationType { LINEAR, STEP, CUBICSPLINE };
	InterpolationType interpolation;
	std::vector<float> inputs;
	std::vector<V4> outputsVec4;
};

struct Animation {
	String name;
	std::vector<AnimationSampler> samplers;
	std::vector<AnimationChannel> channels;
	float start = std::numeric_limits<float>::max();
	float end = std::numeric_limits<float>::min();
};

template<typename T> struct SBufferReadData {
	const uint8_t* SrcData = nullptr;
	size_t SrcStride = 0;

	std::vector<T> DstData;
	size_t DstStride = 0;

	ShaderDataType _shader_data_type;
	size_t NumElements = 0;
};

template<typename T> T mix(const T& x, const T& y, float a)
{
	return x * (1.0 - a) + y * a;
}

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

template<typename T, typename ComponentType = void> SBufferReadData<T> getAttributeData(const tinygltf::Model& model, uint32_t attributeValue) {
    const tinygltf::Accessor& accessor = model.accessors.at(attributeValue);
    const tinygltf::BufferView& bufferView = model.bufferViews.at(accessor.bufferView);
    const tinygltf::Buffer& buffer = model.buffers.at(bufferView.buffer);
    const ShaderDataType shaderDataType = getShaderDataTypeFromAccessor(accessor);

    size_t componentSize = 4;
    if constexpr(!std::is_same_v<ComponentType, void>)
        componentSize = sizeof(ComponentType);

    SBufferReadData<T> bufferReadParams = getBufferReadData<T>(&accessor, bufferView, buffer, shaderDataType, componentSize);

    readBufferData<T, ComponentType>(bufferReadParams);
    return bufferReadParams;
}

template<typename T> sp<Array<T>> toArray(std::vector<T> vector) {
    return vector.size() ? sp<typename Array<T>::Vector>::make(std::move(vector)) : nullptr;
}

Mesh processPrimitive(const tinygltf::Model& gltfModel, const std::vector<sp<Material>>& materials, const tinygltf::Primitive& primitive, uint32_t id, String name) {
    std::vector<V3> vertices;
    std::vector<V3> normals;
    std::vector<Mesh::UV> uvs;

    for(const auto& [attributeKey, attributeValue] : primitive.attributes)
    {
        if(attributeKey == "POSITION") {
            SBufferReadData bufferReadData = getAttributeData<V3>(gltfModel, attributeValue);
            vertices = std::move(bufferReadData.DstData);
        }
        else if(attributeKey == "NORMAL") {
            SBufferReadData bufferReadData = getAttributeData<V3>(gltfModel, attributeValue);
            normals = std::move(bufferReadData.DstData);
        }
        else if(attributeKey == "TEXCOORD_0") {
            SBufferReadData bufferReadData = getAttributeData<V2>(gltfModel, attributeValue);
            uvs.reserve(bufferReadData.DstData.size());
            for(const V2& i : bufferReadData.DstData)
                uvs.emplace_back(Atlas::unnormalize(i.x()), Atlas::unnormalize(i.y()));
        }
        else {
            WARN("Ignoring primitive attribute \"%s\"", attributeKey.c_str());
        }
    }

    SBufferReadData bufferReadData = getAttributeData<element_index_t, element_index_t>(gltfModel, primitive.indices);
    std::vector<element_index_t> indices = std::move(bufferReadData.DstData);

    ASSERT(primitive.material == -1 || primitive.material < materials.size());
    sp<Material> material = primitive.material >= 0 ? materials.at(primitive.material) : nullptr;

    return {id, std::move(name), std::move(indices), std::move(vertices), toArray<Mesh::UV>(std::move(uvs)), toArray<V3>(std::move(normals)), nullptr, nullptr, std::move(material)};
}

M4 getNodeLocalTransformMatrix(const tinygltf::Node& node)
{
	V3 translation(0);
	if(!node.translation.empty())
		translation = V3(static_cast<float>(node.translation.at(0)), static_cast<float>(node.translation.at(1)), static_cast<float>(node.translation.at(2)));

	V3 scale(1.0f);
	if(!node.scale.empty())
		scale = V3(static_cast<float>(node.scale.at(0)), static_cast<float>(node.scale.at(1)), static_cast<float>(node.scale.at(2)));

	V4 quaternion(constants::QUATERNION_ZERO);
	if(!node.rotation.empty())
		quaternion = V4(static_cast<float>(node.rotation.at(0)), static_cast<float>(node.rotation.at(1)), static_cast<float>(node.rotation.at(2)), static_cast<float>(node.rotation.at(3)));

	const M4 translateMatrix = MatrixUtil::translate(M4::identity(), translation);
	const M4 rotateMatrix = MatrixUtil::rotate(M4::identity(), quaternion);
	const M4 scaleMatrix = MatrixUtil::scale(M4::identity(), scale);
	return translateMatrix * rotateMatrix * scaleMatrix;
}

std::vector<sp<Material>> loadMaterials(tinygltf::Model const& gltfModel, const MaterialBundle& materialBundle)
{
	std::vector<sp<Material>> materials(gltfModel.materials.size());

	for(size_t i = 0; i < gltfModel.materials.size(); ++i) {
		const tinygltf::Material& gltfMaterial = gltfModel.materials.at(i);
		String mName = gltfMaterial.name;
		sp<Material>& material = materials[i];
		if(!(material = materialBundle.getMaterial(mName))) {
			material = sp<Material>::make(i, std::move(mName));
			if(gltfMaterial.pbrMetallicRoughness.baseColorTexture.index == -1) {
				const std::vector<double>& vertexColorData = gltfMaterial.pbrMetallicRoughness.baseColorFactor;
				V4 vertexColor(static_cast<float>(vertexColorData.at(0)), static_cast<float>(vertexColorData.at(1)),
							   static_cast<float>(vertexColorData.at(2)), static_cast<float>(vertexColorData.at(3)));
				material->baseColor()->setColor(sp<Vec4>::make<Vec4::Const>(vertexColor));
			}
			material->roughness()->setColor(sp<Vec4>::make<Vec4::Const>(V4(static_cast<float>(gltfMaterial.pbrMetallicRoughness.roughnessFactor), 0, 0, 0)));
			material->metallic()->setColor(sp<Vec4>::make<Vec4::Const>(V4(static_cast<float>(gltfMaterial.pbrMetallicRoughness.metallicFactor), 0, 0, 0)));

			const std::vector<double>& emission = gltfMaterial.emissiveFactor;
			material->emission()->setColor(sp<Vec4>::make<Vec4::Const>(V4(static_cast<float>(emission.at(0)), static_cast<float>(emission.at(1)), static_cast<float>(emission.at(2)), 0)));
			// TODO: Normals
		}
	}

	return materials;
}

tinygltf::Model loadGltfModel(const String& src)
{
	tinygltf::TinyGLTF loader;
	tinygltf::Model gltfModel;
	std::string errString, warnString;
	const sp<Readable> readable = Ark::instance().openAsset(src);

	std::vector<uint8_t> buf(readable->remaining());
	readable->read(buf.data(), static_cast<uint32_t>(buf.size()));
	if(src.endsWith(".gltf"))
		loader.LoadASCIIFromString(&gltfModel, &errString, &warnString, reinterpret_cast<const char*>(buf.data()), buf.size(), "");
	else
		loader.LoadBinaryFromMemory(&gltfModel, &errString, &warnString, buf.data(), buf.size());

	CHECK(errString.empty(), "Loading \"%s\" failed with error: %s", src.c_str(), errString.c_str());
	CHECK_WARN(warnString.empty(), "Warning: \"%s\" %s", src.c_str(), warnString.c_str());

	CHECK(gltfModel.scenes.size() == 1, "\"%s\" must have only one scene, but this file has %d", src.c_str(), gltfModel.scenes.size());

	std::set<HashId> nameHashes;
	for(const tinygltf::Node& i : gltfModel.nodes)
	{
		HashId nameHash = string_hash(i.name.c_str());
		CHECK(nameHashes.find(nameHash) == nameHashes.end(), "Duplicated node \"%s\" found in \"%s\"", i.name.c_str(), src.c_str());
		nameHashes.insert(nameHash);
	}

	return gltfModel;
}

Animation loadAnimation(const tinygltf::Model& model, const tinygltf::Animation& anim)
{
    Animation animation;

	for (const auto& i : anim.samplers)
	{
		AnimationSampler sampler{};

		if(i.interpolation == "LINEAR")
			sampler.interpolation = AnimationSampler::InterpolationType::LINEAR;
		else if(i.interpolation == "STEP")
			sampler.interpolation = AnimationSampler::InterpolationType::STEP;
		else if(i.interpolation == "CUBICSPLINE")
			sampler.interpolation = AnimationSampler::InterpolationType::CUBICSPLINE;

		// Read sampler input time values
		{
			const tinygltf::Accessor& accessor = model.accessors[i.input];
			const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
			const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];

			assert(accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);

			float* buf = new float[accessor.count];
			memcpy(buf, &buffer.data[accessor.byteOffset + bufferView.byteOffset], accessor.count * sizeof(float));
			for (size_t index = 0; index < accessor.count; index++)
				sampler.inputs.push_back(buf[index]);
            delete[] buf;
			for(const float input : sampler.inputs)
			{
				if(input < animation.start)
					animation.start = input;
				if(input > animation.end)
					animation.end = input;
			}
		}

		// Read sampler output T/R/S values
		{
			const tinygltf::Accessor& accessor = model.accessors[i.output];
			const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
			const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];

			assert(accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);

			switch(accessor.type) {
				case TINYGLTF_TYPE_VEC3: {
					std::vector<V3> buf(accessor.count);
					memcpy(buf.data(), &buffer.data[accessor.byteOffset + bufferView.byteOffset], accessor.count * sizeof(V3));
					for (size_t index = 0; index < accessor.count; index++)
						sampler.outputsVec4.push_back(V4(buf.at(index), 0.0f));
	                break;
				}
				case TINYGLTF_TYPE_VEC4: {
					std::vector<V4> buf(accessor.count);
					memcpy(buf.data(), &buffer.data[accessor.byteOffset + bufferView.byteOffset], accessor.count * sizeof(V4));
					for (size_t index = 0; index < accessor.count; index++)
						sampler.outputsVec4.push_back(buf.at(index));
	                break;
				}
				default:
					FATAL("Unknown type %d", accessor.type);
					break;
			}
		}

		animation.samplers.push_back(sampler);
	}

	for (const tinygltf::AnimationChannel& source: anim.channels)
		if(source.target_node >= 0)
		{
			AnimationChannel channel{};

			if(source.target_path == "rotation") {
				channel.path = AnimationChannel::PathType::ROTATION;
			}
			else if(source.target_path == "translation") {
				channel.path = AnimationChannel::PathType::TRANSLATION;
			}
			else if(source.target_path == "scale") {
				channel.path = AnimationChannel::PathType::SCALE;
			}
			CHECK(source.target_path != "weights", "Implemented");

			channel.samplerIndex = source.sampler;
			channel.node_id = source.target_node;
			animation.channels.push_back(channel);
		}

	return animation;
}

void updateAnimation(Animation& animation, float time)
{
	for (auto& channel : animation.channels)
	{
		const AnimationSampler& sampler = animation.samplers[channel.samplerIndex];
		if(sampler.inputs.size() > sampler.outputsVec4.size())
			continue;

		for (auto i = 0; i < sampler.inputs.size() - 1; i++)
			if ((time >= sampler.inputs[i]) && (time <= sampler.inputs[i + 1]))
				if(const float u = std::max(0.0f, time - sampler.inputs[i]) / (sampler.inputs[i + 1] - sampler.inputs[i]); u <= 1.0f) {
					switch (channel.path) {
						case AnimationChannel::PathType::TRANSLATION: {
							channel._intermediate_translation = mix<V4>(sampler.outputsVec4[i], sampler.outputsVec4[i + 1], u).toNonHomogeneous();
							break;
						}
						case AnimationChannel::PathType::SCALE: {
							channel._intermediate_scale = mix<V4>(sampler.outputsVec4[i], sampler.outputsVec4[i + 1], u).toNonHomogeneous();
							break;
						}
						case AnimationChannel::PathType::ROTATION: {
							channel._intermediate_quaternion = Math::slerp(sampler.outputsVec4[i], sampler.outputsVec4[i + 1], u);
							break;
						}
					}
				}
	}
}	

}

GltfImporter::GltfImporter(const String& src, const MaterialBundle& materialBundle)
	: _model(new tinygltf::Model(loadGltfModel(src))), _materials(loadMaterials(_model, materialBundle)), _nodes(_model->nodes.size()), _primitives_in_mesh(_model->meshes.size())
{
}

void GltfImporter::loadPrimitives()
{
	uint32_t meshId = 0, primitiveId = 0;
	for(const tinygltf::Mesh& i : _model->meshes) {
		uint32_t primitiveBase = 0;
		std::vector<uint32_t> primitiveIds;
		for(const tinygltf::Primitive& j : i.primitives) {
			String primitiveName = i.primitives.size() == 1 ? String(i.name) : Strings::sprintf("%s-%d", i.name.c_str(), primitiveBase++);
			primitiveIds.push_back(primitiveId);
			_primitives.push_back(sp<Mesh>::make(processPrimitive(_model, _materials, j, primitiveId++, std::move(primitiveName))));
		}
		_primitives_in_mesh[meshId++] = std::move(primitiveIds);
	}
}

Model GltfImporter::loadModel()
{
	const tinygltf::Scene& scene = _model->scenes.at(0);
	sp<Node> rootNode = sp<Node>::make(scene.name, M4::identity());

	for(const int32_t i : scene.nodes)
		rootNode->childNodes().push_back(loadNode(i));

	Table<String, sp<ark::Animation>> animations;
	if(!_model->animations.empty())
	{
		const float tps = 24.0f;
		std::vector<Animation> loadingAnimations;
		float animationDuration = std::numeric_limits<float>::min();

		for(const tinygltf::Animation& i : _model->animations)
		{
			Animation animation = loadAnimation(_model, i);
			if(animationDuration < animation.end)
				animationDuration = animation.end;
			loadingAnimations.push_back(std::move(animation));
		}

		const uint32_t tickCount = animationDuration * tps;
		const float tickInterval = 1.0f / tps;

		for(Animation& animation : loadingAnimations)
		{
			std::map<uint32_t, uint32_t> channelNodeIds;
			for(const AnimationChannel& channel : animation.channels)
				channelNodeIds.emplace(channel.node_id, channelNodeIds.size());

			std::vector<AnimationFrame> frames(tickCount);

			for(uint32_t i = 0; i < tickCount; ++i)
			{
				AnimationFrame animationFrame(channelNodeIds.size());

				const float time = i * tickInterval;
				updateAnimation(animation, time);
				for(const AnimationChannel& channel : animation.channels)
				{
					const uint32_t frameNodeId = channelNodeIds[channel.node_id];
					const M4 matrix = MatrixUtil::translate(MatrixUtil::rotate(MatrixUtil::scale(M4::identity(), channel._intermediate_scale), channel._intermediate_quaternion), channel._intermediate_translation);
					animationFrame[frameNodeId] = MatrixUtil::mul(matrix, animationFrame.at(frameNodeId));
				}

				frames[i] = std::move(animationFrame);
			}

			Table<String, uint32_t> nodeIds;
			std::vector<String> nodeNames;
			for(const auto [nodeId, frameNodeId] : channelNodeIds)
			{
				const sp<Node>& node = _nodes.at(nodeId);
				nodeNames.push_back(node->name());
				nodeIds.push_back(node->name(), frameNodeId);
			}

			animations.push_back(std::move(animation.name), sp<ark::Animation>::make<AnimationGltf>(tickCount, std::move(nodeNames), std::move(nodeIds), std::move(frames)));
		}
	}

	float aabbMinX(std::numeric_limits<float>::max()), aabbMinY(std::numeric_limits<float>::max()), aabbMinZ(std::numeric_limits<float>::max());
	float aabbMaxX(std::numeric_limits<float>::min()), aabbMaxY(std::numeric_limits<float>::min()), aabbMaxZ(std::numeric_limits<float>::min());
	for(const sp<Mesh>& i : _primitives)
		for(const V3& j : i->vertices())
		{
			if(aabbMinX > j.x()) aabbMinX = j.x();
			if(aabbMinY > j.y()) aabbMinY = j.y();
			if(aabbMinZ > j.z()) aabbMinZ = j.z();

			if(aabbMaxX < j.x()) aabbMaxX = j.x();
			if(aabbMaxY < j.y()) aabbMaxY = j.y();
			if(aabbMaxZ < j.z()) aabbMaxZ = j.z();
		}

	return {std::move(_materials), std::move(_primitives), std::move(rootNode), sp<Boundaries>::make(V3(aabbMinX, aabbMinY, aabbMinZ), V3(aabbMaxX, aabbMaxY, aabbMaxZ)), nullptr, std::move(animations)};
}

sp<Node> GltfImporter::loadNode(int32_t nodeId)
{
	const tinygltf::Node& node = _model->nodes.at(nodeId);
	sp<Node> n = sp<Node>::make(node.name, getNodeLocalTransformMatrix(node));
	_nodes[nodeId] = n;

	if(node.mesh != -1)
	{
		ASSERT(node.mesh < _primitives_in_mesh.size());
		for(const uint32_t i : _primitives_in_mesh.at(node.mesh))
			n->meshes().push_back(_primitives.at(i));
	}

	for(const int32_t i : node.children)
		n->childNodes().push_back(loadNode(i));
	return n;
}

}
