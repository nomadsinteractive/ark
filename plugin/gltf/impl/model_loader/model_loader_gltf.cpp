#include "plugin/gltf/impl/model_loader/model_loader_gltf.h"

#include "gltf/gltf_importer.h"

#include "renderer/base/material_bundle.h"

namespace ark::plugin::gltf {

namespace {

}

Model ModelImporterGltf::import(const Manifest& manifest, MaterialBundle& materialBundle)
{
    GltfImporter importer(manifest.src(), materialBundle);
    importer.loadPrimitives();
    return importer.loadModel();
}

sp<ModelLoader::Importer> ModelImporterGltf::BUILDER::build(const Scope& args)
{
    return sp<ModelImporterGltf>::make();
}

}
