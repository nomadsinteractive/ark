#ifndef ARK_GRAPHICS_BASE_RECT_H_
#define ARK_GRAPHICS_BASE_RECT_H_

#include <map>

#include "core/base/api.h"

namespace ark {

class ARK_API Rect {
public:
    _CONSTEXPR Rect() noexcept
        : _left(0.0f), _top(0.0f), _right(0.0f), _bottom(0.0f) {
    }
    Rect(const Rect& other) noexcept;
    Rect(float left, float top, float right, float bottom) noexcept;

    float left() const;
    void setLeft(float left);

    float top() const;
    void setTop(float top);

    float right() const;
    void setRight(float right);

    float bottom() const;
    void setBottom(float bottom);

    float width() const;
    float height() const;

    void scale(float xScale, float yScale);
    void translate(float x, float y);
    bool ptin(float x, float y) const;
    bool overlap(const Rect& other) const;

    void setCenter(float x, float y);

    static Rect parse(const std::map<String, String>& properties);
    static Rect parse(const document& manifest);

private:
    float _left;
    float _top;
    float _right;
    float _bottom;
};

}
#endif
