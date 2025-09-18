#include "plugin/gltf/impl/model_loader/model_loader_gltf.h"

#include "core/base/manifest.h"

#include "renderer/base/material_bundle.h"
#include "renderer/base/model.h"

#include "gltf/gltf_importer.h"

namespace ark::plugin::gltf {

Model ModelImporterGltf::import(const Manifest& manifest, MaterialBundle::Initializer& materialInitializer)
{
    GltfImporter importer(manifest.src(), materialInitializer);
    return importer.loadModel();
}

sp<ModelImporter> ModelImporterGltf::BUILDER::build(const Scope& args)
{
    return sp<ModelImporter>::make<ModelImporterGltf>();
}

}
