#pragma once

#include "renderer/base/material_bundle.h"

namespace ark {

class ARK_API ModelImporter {
public:
    virtual ~ModelImporter() = default;

    [[nodiscard]]
    virtual Model import(const Manifest& manifest, MaterialBundle::Initializer& materialInitializer) = 0;
};

}