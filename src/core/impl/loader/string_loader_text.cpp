#include "core/impl/loader/string_loader_text.h"

#include "core/base/json.h"
#include "core/util/strings.h"

namespace ark {

String StringLoaderText::load(const sp<Readable>& readable)
{
    return Strings::loadFromReadable(readable);
}

}
