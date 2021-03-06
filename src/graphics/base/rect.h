#ifndef ARK_GRAPHICS_BASE_RECT_H_
#define ARK_GRAPHICS_BASE_RECT_H_

#include "core/base/api.h"
#include "core/util/documents.h"

#include "graphics/forwarding.h"
#include "graphics/base/v4.h"

namespace ark {

template<typename T> class RectT {
public:
    _CONSTEXPR RectT() noexcept
        : _left(0), _top(0), _right(0), _bottom(0) {
    }
    RectT(T left, T top, T right, T bottom) noexcept
        : _left(left), _top(top), _right(right), _bottom(bottom) {
    }
    template<typename U> RectT(const RectT<U>& other) noexcept
        : _left(static_cast<T>(other.left())), _top(static_cast<T>(other.top())), _right(static_cast<T>(other.right())), _bottom(static_cast<T>(other.bottom())) {
    }
    RectT(const V4& vec4) noexcept
        : _left(static_cast<T>(vec4.x())), _top(vec4.y()), _right(vec4.z()), _bottom(vec4.w()) {
    }
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(RectT);

    T left() const {
        return _left;
    }
    void setLeft(T left) {
        _left = left;
    }

    T top() const {
        return _top;
    }
    void setTop(T top) {
        _top = top;
    }

    T right() const {
        return _right;
    }
    void setRight(T right) {
        _right = right;
    }

    T bottom() const {
        return _bottom;
    }
    void setBottom(T bottom) {
        _bottom = bottom;
    }

    T width() const {
        return std::abs(_right - _left);
    }
    T height() const {
        return std::abs(_bottom - _top);
    }

    void scale(T xScale, T yScale) {
        _left = _left * xScale;
        _right = _right * xScale;
        _top = _top * yScale;
        _bottom = _bottom * yScale;
    }
    void scale(const V2& scale) {
        this->scale(static_cast<T>(scale.x()), static_cast<T>(scale.y()));
    }
    void translate(T x, T y) {
        _left += x;
        _right += x;
        _top += y;
        _bottom += y;
    }

    void vflip(T height) {
        std::swap(_top, _bottom);
        _top = height - _top;
        _bottom = height - _bottom;
    }

    bool ptin(T x, T y) const {
        if(_left <= _right && (x < _left || x > _right))
            return false;
        if(_left > _right && (x > _left || x < _right))
            return false;
        if(_top <= _bottom && (y < _top || y > _bottom))
            return false;
        if(_top > _bottom && (y > _top || y < _bottom))
            return false;
        return true;
    }
    bool overlaps(const RectT& other) const {
        return _left < other._right && _right > other._left && _top < other._bottom && _bottom > other._top;
    }

    void setCenter(T x, T y) {
        float dx = x - (_left + _right) / 2;
        float dy = y - (_top + _bottom) / 2;
        _left += dx;
        _right += dx;
        _top += dy;
        _bottom += dy;
    }

    static RectT parse(const document& manifest) {
        const String rect = Documents::getAttribute(manifest, Constants::Attributes::RECT);
        if(rect)
            return Strings::parse<RectT>(rect);

        const RectT bounds = Strings::parse<RectT>(Documents::ensureAttribute(manifest, Constants::Attributes::BOUNDS));
        return RectT(bounds.left(), bounds.top(), bounds.left() + bounds.right(), bounds.top() + bounds.bottom());
    }


private:
    T _left;
    T _top;
    T _right;
    T _bottom;
};

}
#endif
