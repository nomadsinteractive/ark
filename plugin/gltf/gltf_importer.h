#pragma once

#include "core/collection/table.h"
#include "core/types/owned_ptr.h"

#include "graphics/base/material.h"

#include "renderer/forwarding.h"

namespace tinygltf {

struct Animation;
class Model;

}

namespace ark::plugin::gltf {

class GltfImporter {
public:
	GltfImporter(const String& src, const MaterialBundle& materialBundle);
	~GltfImporter();

	sp<Node> loadNode(WeakPtr<Node> parentNode, int32_t nodeId);
	void loadPrimitives();

	Model loadModel();

private:
	op<tinygltf::Model> _model;
	std::vector<sp<Material>> _materials;

	std::vector<sp<Node>> _nodes;
	std::vector<sp<Mesh>> _primitives;
	std::vector<std::vector<uint32_t>> _primitives_in_mesh;

	Table<String, sp<Animation>> _animations;
};

}
