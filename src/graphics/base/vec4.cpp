#include "graphics/base/vec4.h"

#include <algorithm>

#include "core/base/bean_factory.h"
#include "core/base/object_pool.h"
#include "core/base/variable_wrapper.h"
#include "core/impl/numeric/translate.h"
#include "core/util/bean_utils.h"
#include "core/util/numeric_util.h"
#include "core/util/strings.h"

#include "graphics/base/color.h"
#include "graphics/base/v4.h"

namespace ark {

namespace {

class VV4X : public Numeric {
public:
    VV4X(const sp<VV4>& vv4)
        : _vv4(vv4) {
    }

    virtual float val() override {
        const V3 v3 = _vv4->val();
        return v3.x();
    }

private:
    sp<VV4> _vv4;
};


class VV4Y : public Numeric {
public:
    VV4Y(const sp<VV4>& vv4)
        : _vv4(vv4) {
    }

    virtual float val() override {
        const V3 v3 = _vv4->val();
        return v3.y();
    }

private:
    sp<VV4> _vv4;
};

class VV4Z : public Numeric {
public:
    VV4Z(const sp<VV4>& vv4)
        : _vv4(vv4) {
    }

    virtual float val() override {
        const V3 v3 = _vv4->val();
        return v3.z();
    }

private:
    sp<VV4> _vv4;
};

class VV4W : public Numeric {
public:
    VV4W(const sp<VV4>& vv4)
        : _vv4(vv4) {
    }

    virtual float val() override {
        const V4 v3 = _vv4->val();
        return v3.z();
    }

private:
    sp<VV4> _vv4;
};


}

Vec4::Vec4() noexcept
    : _x(sp<NumericWrapper>::make(0.0f)), _y(sp<NumericWrapper>::make(0.0f)), _z(sp<NumericWrapper>::make(0.0f)), _w(sp<NumericWrapper>::make(0.0f))
{
}

Vec4::Vec4(float x, float y, float z, float w) noexcept
    : _x(sp<NumericWrapper>::make(x)), _y(sp<NumericWrapper>::make(y)), _z(sp<NumericWrapper>::make(z)), _w(sp<NumericWrapper>::make(w))
{
}

Vec4::Vec4(const sp<Numeric>& x, const sp<Numeric>& y, const sp<Numeric>& z, const sp<Numeric>& w) noexcept
    : _x(sp<NumericWrapper>::make(x)), _y(sp<NumericWrapper>::make(y)), _z(sp<NumericWrapper>::make(z)), _w(sp<NumericWrapper>::make(w))
{
}

Vec4::Vec4(const sp<VV4>& delegate) noexcept
    : Vec4()
{
    setDelegate(delegate);
}

V4 Vec4::val()
{
    return V4(_x->val(), _y->val(), _z->val(), _w->val());
}

Vec4 operator +(const Vec4& lvalue, const Vec4& rvalue)
{
    return Vec4(NumericUtil::add(lvalue._x, rvalue._x), NumericUtil::add(lvalue._y, rvalue._y), NumericUtil::add(lvalue._z, rvalue._z), NumericUtil::add(lvalue._w, rvalue._w));
}

Vec4 operator -(const Vec4& lvalue, const Vec4& rvalue)
{
    return Vec4(NumericUtil::sub(lvalue._x, rvalue._x), NumericUtil::sub(lvalue._y, rvalue._y), NumericUtil::sub(lvalue._z, rvalue._z), NumericUtil::sub(lvalue._w, rvalue._w));
}

Vec4 operator *(const Vec4& lvalue, const Vec4& rvalue)
{
    return Vec4(NumericUtil::mul(lvalue._x, rvalue._x), NumericUtil::mul(lvalue._y, rvalue._y), NumericUtil::mul(lvalue._z, rvalue._z), NumericUtil::mul(lvalue._w, rvalue._w));
}

Vec4 operator /(const Vec4& lvalue, const Vec4& rvalue)
{
    return Vec4(NumericUtil::truediv(lvalue._x, rvalue._x), NumericUtil::truediv(lvalue._y, rvalue._y), NumericUtil::truediv(lvalue._z, rvalue._z), NumericUtil::truediv(lvalue._w, rvalue._w));
}

sp<Vec4> Vec4::negative()
{
    return sp<Vec4>::make(NumericUtil::negative(_x), NumericUtil::negative(_y), NumericUtil::negative(_z), NumericUtil::negative(_w));
}

float Vec4::x() const
{
    return _x->val();
}

float Vec4::y() const
{
    return _y->val();
}

float Vec4::z() const
{
    return _z->val();
}

float Vec4::w() const
{
    return _w->val();
}

void Vec4::setX(float x)
{
    _x->set(x);
}

void Vec4::setY(float y)
{
    _y->set(y);
}

void Vec4::setZ(float z)
{
    _z->set(z);
}

void Vec4::setW(float w)
{
    _w->set(w);
}

sp<Numeric> Vec4::vx() const
{
    return _x;
}

sp<Numeric> Vec4::vy() const
{
    return _y;
}

sp<Numeric> Vec4::vz() const
{
    return _z;
}

sp<Numeric> Vec4::vw() const
{
    return _w;
}

void Vec4::setVx(const sp<Numeric>& vx) const
{
    _x->set(vx);
}

void Vec4::setVy(const sp<Numeric>& vy) const
{
    _y->set(vy);
}

void Vec4::setVz(const sp<Numeric>& vz) const
{
    _z->set(vz);
}

void Vec4::setVw(const sp<Numeric>& vw) const
{
    _w->set(vw);
}

void Vec4::setDelegate(const sp<VV4>& delegate)
{
    if(delegate)
    {
        if(delegate.is<Vec4>())
        {
            const sp<Vec4> vec4 = delegate.cast<Vec4>();
            _x->set(vec4->_x);
            _y->set(vec4->_y);
            _z->set(vec4->_z);
            _w->set(vec4->_w);
        }
        else
        {
            _x->set(sp<VV4X>::make(delegate));
            _y->set(sp<VV4Y>::make(delegate));
            _z->set(sp<VV4Z>::make(delegate));
            _w->set(sp<VV4W>::make(delegate));
        }
    }
    else
    {
        _x->set(nullptr);
        _y->set(nullptr);
        _z->set(nullptr);
        _w->set(nullptr);
    }
}

Vec4::BUILDER::BUILDER(BeanFactory& parent, const document& doc)
    : _x(parent.getBuilder<Numeric>(doc, "x")), _y(parent.getBuilder<Numeric>(doc, "y")), _z(parent.getBuilder<Numeric>(doc, "z")), _w(parent.getBuilder<Numeric>(doc, "w"))
{
}

sp<Vec4> Vec4::BUILDER::build(const sp<Scope>& args)
{
    return sp<Vec4>::make(_x->build(args), _y->build(args), _z->build(args), _w->build(args));
}

template<> ARK_API const sp<Vec4>& Null::ptr()
{
    static sp<Vec4> instance = sp<Vec4>::make();
    return instance;
}

Vec4::DICTIONARY::DICTIONARY(BeanFactory& parent, const String& str)
{
    if(str.find('@') != String::npos || str.find('$') != String::npos)
    {
        _xyzw = BeanUtils::split<Numeric>(parent, str);
        DCHECK(_xyzw->length() == 4, "Index out of range");
    }
    else
        _v4 = str.at(0) == '#' ? Strings::parse<Color>(str).val() : Strings::parse<V4>(str);
}

sp<VV4> Vec4::DICTIONARY::build(const sp<Scope>& args)
{
    if(_xyzw)
    {
        auto s = _xyzw->array();
        return sp<Vec4>::make(s[0]->build(args), s[1]->build(args), s[2]->build(args), s[3]->build(args));
    }
    return sp<VV4::Impl>::make(_v4);
}

}
