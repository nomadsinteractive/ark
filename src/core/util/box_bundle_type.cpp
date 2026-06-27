#include "box_bundle_type.h"

#include "core/base/string.h"

#include "core/inf/dictionary.h"
#include "core/types/box.h"

namespace ark {

Box BoxBundleType::get(BoxBundle& self, const String& name)
{
    return self.get(name);
}

Vector<String> BoxBundleType::keys(BoxBundle& self)
{
    return self.keys();
}

Vector<Box> BoxBundleType::values(BoxBundle& self)
{
    return self.values();
}

}
