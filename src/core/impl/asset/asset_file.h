#pragma once

#include "core/base/string.h"
#include "core/inf/asset.h"

namespace ark {

class AssetFile final : public Asset {
public:
    AssetFile(String filepath);

    sp<Readable> open() override;
    String location() override;

private:
    String _filepath;
};

}
