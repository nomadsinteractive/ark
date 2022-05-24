#include "core/impl/loader/json_loader_text.h"

#include "core/base/json.h"
#include "core/util/strings.h"

namespace ark {

Json JsonLoaderText::load(const sp<Readable>& readable)
{
    Json json;
    json.load(Strings::loadFromReadable(readable));
    return json;
}

}
