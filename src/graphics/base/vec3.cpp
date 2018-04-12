#include "graphics/base/vec3.h"

#include <algorithm>

#include "core/ark.h"
#include "core/base/bean_factory.h"
#include "core/base/object_pool.h"
#include "core/base/variable_wrapper.h"
#include "core/impl/numeric/translate.h"
#include "core/util/bean_utils.h"
#include "core/util/numeric_util.h"
#include "core/util/strings.h"

#include "graphics/base/v3.h"
#include "graphics/base/vec2.h"

namespace ark {

namespace {

class VV3X : public Numeric {
public:
    VV3X(const sp<VV3>& vv3)
        : _vv3(vv3) {
    }

    virtual float val() override {
        const V3 v3 = _vv3->val();
        return v3.x();
    }

private:
    sp<VV3> _vv3;
};


class VV3Y : public Numeric {
public:
    VV3Y(const sp<VV3>& vv3)
        : _vv3(vv3) {
    }

    virtual float val() override {
        const V3 v3 = _vv3->val();
        return v3.y();
    }

private:
    sp<VV3> _vv3;
};

class VV3Z : public Numeric {
public:
    VV3Z(const sp<VV3>& vv3)
        : _vv3(vv3) {
    }

    virtual float val() override {
        const V3 v3 = _vv3->val();
        return v3.z();
    }

private:
    sp<VV3> _vv3;
};

}

Vec3::Vec3() noexcept
    : _x(sp<NumericWrapper>::make(0.0f)), _y(sp<NumericWrapper>::make(0.0f)), _z(sp<NumericWrapper>::make(0.0f))
{
}

Vec3::Vec3(float x, float y, float z) noexcept
    : _x(sp<NumericWrapper>::make(x)), _y(sp<NumericWrapper>::make(y)), _z(sp<NumericWrapper>::make(z))
{
}

Vec3::Vec3(const sp<Numeric>& x, const sp<Numeric>& y, const sp<Numeric>& z) noexcept
    : _x(sp<NumericWrapper>::make(x)), _y(sp<NumericWrapper>::make(y)), _z(sp<NumericWrapper>::make(z))
{
}

Vec3::Vec3(const sp<VV3>& delegate) noexcept
    : Vec3()
{
    setDelegate(delegate);
}

V3 Vec3::val()
{
    return V3(_x->val(), _y->val(), _z->val());
}

Vec3 operator +(const Vec3& lvalue, const Vec3& rvalue)
{
    return Vec3(NumericUtil::add(lvalue._x, rvalue._x), NumericUtil::add(lvalue._y, rvalue._y), NumericUtil::add(lvalue._z, rvalue._z));
}

Vec3 operator -(const Vec3& lvalue, const Vec3& rvalue)
{
    return Vec3(NumericUtil::sub(lvalue._x, rvalue._x), NumericUtil::sub(lvalue._y, rvalue._y), NumericUtil::sub(lvalue._z, rvalue._z));
}

Vec3 operator *(const Vec3& lvalue, const Vec3& rvalue)
{
    return Vec3(NumericUtil::mul(lvalue._x, rvalue._x), NumericUtil::mul(lvalue._y, rvalue._y), NumericUtil::mul(lvalue._z, rvalue._z));
}

Vec3 operator /(const Vec3& lvalue, const Vec3& rvalue)
{
    return Vec3(NumericUtil::truediv(lvalue._x, rvalue._x), NumericUtil::truediv(lvalue._y, rvalue._y), NumericUtil::truediv(lvalue._z, rvalue._z));
}

sp<Vec3> Vec3::negative()
{
    return sp<Vec3>::make(NumericUtil::negative(_x), NumericUtil::negative(_y), NumericUtil::negative(_z));
}

float Vec3::x() const
{
    return _x->val();
}

float Vec3::y() const
{
    return _y->val();
}

float Vec3::z() const
{
    return _z->val();
}

void Vec3::setX(float x)
{
    _x->set(x);
}

void Vec3::setY(float y)
{
    _y->set(y);
}

void Vec3::setZ(float z)
{
    _z->set(z);
}

sp<Numeric> Vec3::vx() const
{
    return _x;
}

sp<Numeric> Vec3::vy() const
{
    return _y;
}

sp<Numeric> Vec3::vz() const
{
    return _z;
}

sp<Vec2> Vec3::vxy() const
{
    return sp<Vec2>::make(_x, _y);
}

void Vec3::setVx(const sp<Numeric>& vx) const
{
    _x->set(vx);
}

void Vec3::setVy(const sp<Numeric>& vy) const
{
    _y->set(vy);
}

void Vec3::setVz(const sp<Numeric>& vz) const
{
    _z->set(vz);
}

void Vec3::setDelegate(const sp<VV3>& delegate)
{
    if(delegate)
    {
        if(delegate.is<Vec3>())
        {
            const sp<Vec3> vec3 = delegate.cast<Vec3>();
            _x->set(vec3->_x);
            _y->set(vec3->_y);
            _z->set(vec3->_z);
        }
        else
        {
            _x->set(sp<VV3X>::make(delegate));
            _y->set(sp<VV3Y>::make(delegate));
            _z->set(sp<VV3Z>::make(delegate));
        }
    }
    else
    {
        _x->set(nullptr);
        _y->set(nullptr);
        _z->set(nullptr);
    }
}

Vec3 Vec3::translate(const Vec3& translation) const
{
    return Vec3(NumericUtil::add(_x, translation._x), NumericUtil::add(_y, translation._y), NumericUtil::add(_z, translation._z));
}

Vec3 Vec3::translate(float x, float y, float z) const
{
    return Vec3(sp<Translate>::make(_x, x), sp<Translate>::make(_y, y), sp<Translate>::make(_z, z));
}

Vec3 Vec3::translate(ObjectPool& op, float x, float y, float z) const
{
    return Vec3(op.obtain<Translate>(_x, x), op.obtain<Translate>(_y, y), op.obtain<Translate>(_z, z));
}

Vec3::BUILDER::BUILDER(BeanFactory& parent, const document& doc)
    : _x(parent.getBuilder<Numeric>(doc, "x")), _y(parent.getBuilder<Numeric>(doc, "y")), _z(parent.getBuilder<Numeric>(doc, "z"))
{
}

sp<Vec3> Vec3::BUILDER::build(const sp<Scope>& args)
{
    return sp<Vec3>::make(_x->build(args), _y->build(args), _z->build(args));
}

template<> ARK_API const sp<Vec3> Null::ptr()
{
    return Ark::instance().obtain<Vec3>();
}

Vec3::DICTIONARY::DICTIONARY(BeanFactory& parent, const String& str)
{
    if(str.find('@') != String::npos || str.find('$') != String::npos)
        _xyz = BeanUtils::split<Numeric>(parent, str);
    else
    {
        const array<float> values = Strings::toArray<float>(str);
        float* ptr = values->buf();
        _v3 = V3(ptr[0], ptr[1], ptr[2]);
    }

}

sp<VV3> Vec3::DICTIONARY::build(const sp<Scope>& args)
{
    if(_xyz)
    {
        auto s = _xyz->buf();
        return sp<Vec3>::make(s[0]->build(args), s[1]->build(args), s[2]->build(args));
    }
    return sp<VV3::Impl>::make(_v3);
}

}
