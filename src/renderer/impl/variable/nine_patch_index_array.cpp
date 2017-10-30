#include "renderer/impl/variable/nine_patch_index_array.h"

#include "core/impl/array/fixed_array.h"

namespace ark {

NinePatchIndexArray::NinePatchIndexArray()
    : _array(indexarray::adopt(new FixedArray<uint16_t, 28>({4, 0, 5, 1, 6, 2, 7, 3, 3, 8, 8, 4, 9, 5, 10, 6, 11, 7, 7, 12, 12, 8, 13, 9, 14, 10, 15, 11})))
{
}

indexarray NinePatchIndexArray::val()
{
    return _array;
}

}
