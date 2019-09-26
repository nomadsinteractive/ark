#include "app/impl/layout/grid_layout.h"

#include "core/base/api.h"
#include "core/util/bean_utils.h"
#include "core/util/documents.h"

#include "graphics/base/size.h"

#include "app/impl/layout/gravity_layout.h"

namespace ark {

GridLayout::GridLayout(uint32_t rows, uint32_t cols, const View::Gravity gravity)
    : _rows(rows), _cols(cols), _gravity(gravity)
{
}

void GridLayout::begin(Context& ctx, LayoutParam& layoutParam)
{
    _grid_width = layoutParam.contentWidth() / _cols;
    _grid_height = layoutParam.contentHeight() / _rows;
    _flow_index = 0;
}

Rect GridLayout::place(Context& ctx, LayoutParam& layoutParam)
{
    DCHECK(_flow_index < _cols * _rows, "Flow index out of bounds");
    Rect rect = GravityLayout::place(_gravity, _grid_width, _grid_height, layoutParam.calcLayoutWidth(_grid_width), layoutParam.calcLayoutHeight(_grid_height));
    rect.translate((_flow_index % _cols) * _grid_width, (_flow_index / _cols) * _grid_height);
    _flow_index ++;
    return rect;
}

Rect GridLayout::end(Context& /*ctx*/)
{
    return Rect();
}

GridLayout::BUILDER::BUILDER(BeanFactory& parent, const document& doc)
    : _rows(parent.ensureBuilder<Numeric>(doc, "rows")), _cols(parent.ensureBuilder<Numeric>(doc, "cols")),
      _gravity(Documents::ensureAttribute<View::Gravity>(doc, Constants::Attributes::GRAVITY))
{
}

sp<Layout> GridLayout::BUILDER::build(const sp<Scope>& args)
{
    return sp<GridLayout>::make(static_cast<uint32_t>(BeanUtils::toFloat(_rows, args)), static_cast<uint32_t>(BeanUtils::toFloat(_cols, args)), _gravity);
}

}
