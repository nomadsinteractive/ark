#pragma once

#include "core/forwarding.h"
#include "core/inf/array.h"
#include "core/inf/builder.h"

#include "renderer/forwarding.h"
#include "renderer/base/material_bundle.h"
#include "renderer/inf/model_importer.h"


namespace ark::plugin::gltf {

class ModelImporterGltf final : public ModelImporter {
public:

    Model import(const Manifest& manifest, MaterialBundle::Initializer& materialInitializer) override;

//  [[plugin::builder::by-value("gltf")]]
    class BUILDER final : public Builder<ModelImporter> {
    public:
        BUILDER() = default;

        sp<ModelImporter> build(const Scope& args) override;
    };

private:
    Vector<sp<Texture>> _textures;
};

}
