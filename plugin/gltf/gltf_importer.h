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
	GltfImporter(const String& src, MaterialBundle& materialBundle);
	~GltfImporter();

	sp<Node> loadNode(WeakPtr<Node> parentNode, int32_t nodeId);
	void loadPrimitives();

	Model loadModel();

private:
	op<tinygltf::Model> _model;
	Vector<sp<Material>> _materials;

	Vector<sp<Node>> _nodes;
	Vector<sp<Mesh>> _primitives;
	Vector<Vector<uint32_t>> _primitives_in_mesh;

	Table<String, sp<Animation>> _animations;
};

}
