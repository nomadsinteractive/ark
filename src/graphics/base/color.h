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
//[[script::bindings::extends(Vec4)]]
class ARK_API Color : public Vec4, public Notifier {
public:
    Color();
//  [[script::bindings::auto]]
    Color(uint32_t value);
    Color(float r, float g, float b, float a);
    Color(float r, float g, float b);
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(Color);

//  [[script::bindings::property]]
    float r() const;
//  [[script::bindings::property]]
    void setR(float r);
//  [[script::bindings::property]]
    float g() const;
//  [[script::bindings::property]]
    void setG(float g);
//  [[script::bindings::property]]
    float b() const;
//  [[script::bindings::property]]
    void setB(float b);
//  [[script::bindings::property]]
    float a() const;
//  [[script::bindings::property]]
    void setA(float a);

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
