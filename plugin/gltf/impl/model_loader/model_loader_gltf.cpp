#include "plugin/gltf/impl/model_loader/model_loader_gltf.h"

#include <vector>

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define TINYGLTF_NOEXCEPTION
#define JSON_NOEXCEPTION
#include <tiny_gltf.h>

#include "core/inf/readable.h"

namespace ark {
namespace plugin {
namespace gltf {

Model ModelImporterGltf::import(const Manifest& manifest, MaterialBundle& materialBundle)
{
    tinygltf::TinyGLTF loader;
    tinygltf::Model model;
    std::string errString, warnString;
    sp<Readable> readable = Ark::instance().openAsset(manifest.src());

    std::vector<uint8_t> buf(readable->remaining());
    readable->read(buf.data(), static_cast<uint32_t>(buf.size()));
    loader.LoadBinaryFromMemory(&model, &errString, &warnString, buf.data(), buf.size());
    return Model();
}

sp<ModelLoader::Importer> ModelImporterGltf::BUILDER::build(const Scope& args)
{
    return sp<ModelImporterGltf>::make();
}

}
}
}
