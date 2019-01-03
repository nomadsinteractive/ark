#ifndef ARK_GRAPHICS_BASE_COLOR_H_
#define ARK_GRAPHICS_BASE_COLOR_H_

#include "core/base/api.h"
#include "core/epi/notifier.h"
#include "core/inf/variable.h"
#include "core/types/class.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/base/v4.h"

namespace ark {

//[[core::class]]
class ARK_API Color : public Vec4, public Notifier {
public:
    Color();
//  [[script::bindings::auto]]
    Color(uint32_t value);
    Color(float red, float green, float blue, float alpha);
    Color(float red, float green, float blue);
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(Color);

//  [[script::bindings::property]]
    float red() const;
//  [[script::bindings::property]]
    void setRed(float red);
//  [[script::bindings::property]]
    float green() const;
//  [[script::bindings::property]]
    void setGreen(float green);
//  [[script::bindings::property]]
    float blue() const;
//  [[script::bindings::property]]
    void setBlue(float blue);
//  [[script::bindings::property]]
    float alpha() const;
//  [[script::bindings::property]]
    void setAlpha(float alpha);

//  [[script::bindings::property]]
    uint32_t value() const;
//  [[script::bindings::property]]
    void setValue(uint32_t value);

//  [[script::bindings::auto]]
    void assign(const Color& other);

    bool operator == (const Color& other) const;
    bool operator != (const Color& other) const;

    static const Color NONE;
    static const Color WHITE;
    static const Color BLACK;

    virtual V4 val() override;

private:
    V4 _color;

};

}
#endif
