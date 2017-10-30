#include "graphics/base/vec3.h"

#include <algorithm>

#include "core/base/bean_factory.h"
#include "core/base/object_pool.h"
#include "core/impl/numeric/scalar.h"
#include "core/impl/numeric/negative.h"
#include "core/impl/numeric/add.h"
#include "core/impl/numeric/divide.h"
#include "core/impl/numeric/multiply.h"
#include "core/impl/numeric/subtract.h"
#include "core/impl/numeric/translate.h"
#include "core/util/bean_utils.h"
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
    : _x(sp<Scalar>::make(0.0f)), _y(sp<Scalar>::make(0.0f)), _z(sp<Scalar>::make(0.0f))
{
}

Vec3::Vec3(float x, float y, float z) noexcept
    : _x(sp<Scalar>::make(x)), _y(sp<Scalar>::make(y)), _z(sp<Scalar>::make(z))
{
}

Vec3::Vec3(const sp<Numeric>& x, const sp<Numeric>& y, const sp<Numeric>& z) noexcept
    : _x(sp<Scalar>::make(x)), _y(sp<Scalar>::make(y)), _z(sp<Scalar>::make(z))
{
}

Vec3::Vec3(const sp<VV3>& delegate) noexcept
    : Vec3()
{
    setDelegate(delegate);
}

Vec3::Vec3(const Vec3& other) noexcept
    : _x(other._x), _y(other._y), _z(other._z)
{
}

Vec3::Vec3(Vec3&& other) noexcept
    : _x(std::move(other._x)), _y(std::move(other._y)), _z(std::move(other._z))
{
}

const Vec3& Vec3::operator =(const Vec3& other)
{
    _x = other._x;
    _y = other._y;
    _z = other._z;
    return *this;
}

const Vec3& Vec3::operator =(Vec3&& other)
{
    _x = std::move(other._x);
    _y = std::move(other._y);
    _z = std::move(other._z);
    return *this;
}

V3 Vec3::val()
{
    return V3(_x->val(), _y->val(), _z->val());
}

Vec3 operator +(const Vec3& lvalue, const Vec3& rvalue)
{
    return Vec3(sp<Add>::make(lvalue._x, rvalue._x), sp<Add>::make(lvalue._y, rvalue._y), sp<Add>::make(lvalue._z, rvalue._z));
}

Vec3 operator -(const Vec3& lvalue, const Vec3& rvalue)
{
    return Vec3(sp<Subtract>::make(lvalue._x, rvalue._x), sp<Subtract>::make(lvalue._y, rvalue._y), sp<Subtract>::make(lvalue._z, rvalue._z));
}

Vec3 operator *(const Vec3& lvalue, const Vec3& rvalue)
{
    return Vec3(sp<Multiply>::make(lvalue._x, rvalue._x), sp<Multiply>::make(lvalue._y, rvalue._y), sp<Multiply>::make(lvalue._z, rvalue._z));
}

Vec3 operator /(const Vec3& lvalue, const Vec3& rvalue)
{
    return Vec3(sp<Divide>::make(lvalue._x, rvalue._x), sp<Divide>::make(lvalue._y, rvalue._y), sp<Divide>::make(lvalue._z, rvalue._z));
}

sp<Vec3> Vec3::negative()
{
    return sp<Vec3>::make(sp<Negative>::make(_x), sp<Negative>::make(_y), sp<Negative>::make(_z));
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
    _x->setDelegate(Null::toSafe<Numeric>(vx));
}

void Vec3::setVy(const sp<Numeric>& vy) const
{
    _y->setDelegate(Null::toSafe<Numeric>(vy));
}

void Vec3::setVz(const sp<Numeric>& vz) const
{
    _z->setDelegate(Null::toSafe<Numeric>(vz));
}

void Vec3::setDelegate(const sp<VV3>& delegate)
{
    if(delegate)
    {
        if(delegate.is<Vec3>())
        {
            const sp<Vec3> vec3 = delegate.cast<Vec3>();
            _x->assign(vec3->_x);
            _y->assign(vec3->_y);
            _z->assign(vec3->_z);
        }
        else
        {
            _x->setDelegate(sp<VV3X>::make(delegate));
            _y->setDelegate(sp<VV3Y>::make(delegate));
            _z->setDelegate(sp<VV3Z>::make(delegate));
        }
    }
    else
    {
        _x->assign(nullptr);
        _y->assign(nullptr);
        _z->assign(nullptr);
    }
}

Vec3 Vec3::translate(const Vec3& translation) const
{
    return Vec3(sp<Add>::make(_x, translation._x), sp<Add>::make(_y, translation._y), sp<Add>::make(_z, translation._z));
}

Vec3 Vec3::translate(float x, float y, float z) const
{
    return Vec3(sp<Translate>::make(_x, x), sp<Translate>::make(_y, y), sp<Translate>::make(_z, z));
}

Vec3 Vec3::translate(ObjectPool<Numeric>& op, float x, float y, float z) const
{
    return Vec3(op.allocate<Translate>(_x, x), op.allocate<Translate>(_y, y), op.allocate<Translate>(_z, z));
}

Vec3::BUILDER::BUILDER(BeanFactory& parent, const document& doc)
    : _x(parent.getBuilder<Numeric>(doc, "x")), _y(parent.getBuilder<Numeric>(doc, "y")), _z(parent.getBuilder<Numeric>(doc, "z"))
{
}

sp<Vec3> Vec3::BUILDER::build(const sp<Scope>& args)
{
    return sp<Vec3>::make(_x->build(args), _y->build(args), _z->build(args));
}

template<> ARK_API const sp<Vec3>& Null::ptr()
{
    static sp<Vec3> instance = sp<Vec3>::make();
    return instance;
}

Vec3::DICTIONARY::DICTIONARY(BeanFactory& parent, const String& str)
{
    if(str.find('@') != String::npos || str.find('$') != String::npos)
        _xyz = BeanUtils::split<Numeric>(parent, str);
    else
    {
        const array<float> values = Strings::toArray<float>(str);
        float* ptr = values->array();
        _v3.setX(ptr[0]);
        _v3.setY(ptr[1]);
        _v3.setZ(ptr[2]);
    }

}

sp<VV3> Vec3::DICTIONARY::build(const sp<Scope>& args)
{
    if(_xyz)
    {
        auto s = _xyz->array();
        return sp<Vec3>::make(s[0]->build(args), s[1]->build(args), s[2]->build(args));
    }
    return sp<VV3::Impl>::make(_v3);
}

}
