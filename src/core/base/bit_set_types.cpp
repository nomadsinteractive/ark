#include "core/base/bit_set_types.h"

#include "core/util/string_convert.h"

namespace ark {

template<> ARK_API enums::UploadStrategy StringConvert::eval<enums::UploadStrategy>(const String& str)
{
    constexpr enums::UploadStrategy::LookupTable<5> table = {{
        {"reload", enums::UPLOAD_STRATEGY_RELOAD},
        {"once", enums::UPLOAD_STRATEGY_ONCE},
        {"on_surface_ready", enums::UPLOAD_STRATEGY_ON_SURFACE_READY},
        {"on_change", enums::UPLOAD_STRATEGY_ON_CHANGE},
        {"on_every_frame", enums::UPLOAD_STRATEGY_ON_EVERY_FRAME}
    }};
    return enums::UploadStrategy::toBitSet(str, table);
}

}