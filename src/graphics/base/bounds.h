#ifndef ARK_GRAPHICS_BASE_BOUNDS_H_
#define ARK_GRAPHICS_BASE_BOUNDS_H_

#include "core/base/api.h"
#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"

#include "graphics/inf/block.h"

namespace ark {

class ARK_API Bounds : public Block {
public:
    Bounds();
    Bounds(const Rect& rect);
    Bounds(float x, float y, float w, float h);
    Bounds(const sp<Vec>& position, const sp<Size>& size);

//  [[script::bindings::auto]]
    bool ptin(float x, float y) const;
//  [[script::bindings::property]]
    const sp<Vec>& position() const;
//  [[script::bindings::property]]
    virtual const sp<Size>& size() override;

//  [[script::bindings::property]]
    float left() const;
//  [[script::bindings::property]]
    float top() const;
//  [[script::bindings::property]]
    float right() const;
//  [[script::bindings::property]]
    float bottom() const;

//  [[script::bindings::property]]
    float width() const;
//  [[script::bindings::property]]
    float height() const;

//  [[plugin::builder::by-value]]
    class DICTIONARY : public Builder<Bounds> {
    public:
        DICTIONARY(BeanFactory& parent, const String& value);

        virtual sp<Bounds> build(const sp<Scope>& args) override;

    private:
        sp<Builder<Vec>> _position;
        sp<Builder<Size>> _size;

        sp<Builder<Numeric>> _x, _y, _w, _h;
    };

private:
    sp<Vec> _position;
    sp<Size> _size;

};

}

#endif
