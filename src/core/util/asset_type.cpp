#include "core/util/asset_type.h"

#include "core/inf/asset.h"
#include "core/types/shared_ptr.h"
#include "core/util/strings.h"

namespace ark {

String AssetType::location(const sp<Asset>& self)
{
    return self->location();
}

String AssetType::readString(const sp<Asset>& self)
{
    return Strings::loadFromReadable(self->open());
}

}
