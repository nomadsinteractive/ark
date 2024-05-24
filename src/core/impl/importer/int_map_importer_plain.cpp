#include "core/impl/importer/int_map_importer_plain.h"

#include "core/util/strings.h"

namespace ark {

void IntMapImporterPlain::import(IntMap& obj, const sp<Readable>& src)
{
    const String content = Strings::loadFromReadable(src);
    for(const String& i : content.split('\n'))
    {
        for(const auto& iter : Strings::parseProperties(i.strip()))
        {
            int32_t key = Strings::eval<int32_t>(iter.first);
            int32_t val = Strings::eval<int32_t>(iter.second);
            obj[key] = val;
        }
    }
}

sp<IntMapImporter> IntMapImporterPlain::BUILDER::build(const Scope& args)
{
    return sp<IntMapImporterPlain>::make();
}

}
