#include "app/impl/layout/grid_layout_v2.h"

#include "core/base/api.h"
#include "core/util/bean_utils.h"
#include "core/util/documents.h"

#include "graphics/base/size.h"


namespace ark {

GridLayoutV2::GridLayoutV2(uint32_t rows, uint32_t cols, const LayoutParam::Gravity gravity)
    : _rows(rows), _cols(cols), _gravity(gravity)
{
}

V2 GridLayoutV2::inflate(const std::vector<sp<LayoutParam>>& slots)
{
    return V2();
}

std::vector<V2> GridLayoutV2::place(const std::vector<sp<LayoutParam>>& slots, const LayoutParam& parent, const V2& contentSize)
{
    std::vector<float> colOffsets;
    for(uint32_t i = 0; i < _rows && i < slots.size(); ++i)
        colOffsets.push_back(i == 0 ? 0 : colOffsets.back() + slots.at(i)->offsetWidth());

    float flowy = 0;
    float flowHeight = 0;
    std::vector<V2> positions;
    for(uint32_t i = 0; i < slots.size(); ++i)
    {
        uint32_t col = i % _rows;
        positions.push_back(V2(colOffsets.at(col), flowy));
        flowHeight = std::max(slots.at(i)->offsetHeight(), flowHeight);
        if(col == _rows - 1)
            flowy += flowHeight;
    }
    return positions;
}

GridLayoutV2::BUILDER::BUILDER(BeanFactory& parent, const document& doc)
    : _rows(parent.ensureBuilder<Numeric>(doc, "rows")), _cols(parent.ensureBuilder<Numeric>(doc, "cols")),
      _gravity(Documents::ensureAttribute<LayoutParam::Gravity>(doc, Constants::Attributes::GRAVITY))
{
}

sp<Layout> GridLayoutV2::BUILDER::build(const Scope& args)
{
    return sp<GridLayoutV2>::make(static_cast<uint32_t>(BeanUtils::toFloat(_rows, args)), static_cast<uint32_t>(BeanUtils::toFloat(_cols, args)), _gravity);
}

}
