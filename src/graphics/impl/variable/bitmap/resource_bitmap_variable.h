#ifndef ARK_GRAPHICS_IMPL_VARIABLE_BITMAP_RESOURCE_BITMAP_VARIABLE_H_
#define ARK_GRAPHICS_IMPL_VARIABLE_BITMAP_RESOURCE_BITMAP_VARIABLE_H_

#include "core/forwarding.h"
#include "core/base/string.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

namespace ark {

class ResourceBitmapVariable : public Variable<bitmap> {
public:
    ResourceBitmapVariable(const sp<Dictionary<bitmap>>& bitmapResource, const String& name);

    virtual bitmap val() override;

private:
    sp<Dictionary<bitmap>> _bitmap_resource;
    String _name;

};

}

#endif
