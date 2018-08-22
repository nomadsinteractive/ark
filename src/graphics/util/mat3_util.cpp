#include "graphics/util/mat3_util.h"

#include <glm/glm.hpp>

#include "core/ark.h"
#include "core/impl/variable/variable_wrapper.h"

#include "graphics/base/mat.h"
#include "graphics/impl/mat/mat3_impl.h"

namespace ark {

namespace {

class Mat3MulMat3 : public Mat3 {
public:
    Mat3MulMat3(const sp<Mat3>& lvalue, const sp<Mat3>& rvalue)
        : _lvalue(lvalue), _rvalue(rvalue) {
    }

    virtual M3 val() override {
        return _lvalue->val().mat<glm::mat3>() * _rvalue->val().mat<glm::mat3>();
    }

private:
    sp<Mat3> _lvalue;
    sp<Mat3> _rvalue;
};

class Mat3MulVec3 : public Vec3 {
public:
    Mat3MulVec3(const sp<Mat3>& lvalue, const sp<Vec3>& rvalue)
        : _lvalue(lvalue), _rvalue(rvalue) {
    }

    virtual V3 val() override {
        const glm::mat3 lvalue = _lvalue->val().mat<glm::mat3>();
        const V3 rvalue = _rvalue->val();
        const glm::vec3 result = lvalue * glm::vec3(rvalue.x(), rvalue.y(), rvalue.z());
        return V3(result.x, result.y, result.z);
    }

private:
    sp<Mat3> _lvalue;
    sp<Vec3> _rvalue;
};

class Mat3MulV3 : public Vec3 {
public:
    Mat3MulV3(const sp<Mat3>& lvalue, const V3& rvalue)
        : _lvalue(lvalue), _rvalue(rvalue.x(), rvalue.y(), rvalue.z()) {
    }

    virtual V3 val() override {
        const glm::mat3 lvalue = _lvalue->val().mat<glm::mat3>();
        const glm::vec3 result = lvalue * _rvalue;
        return V3(result.x, result.y, result.z);
    }

private:
    sp<Mat3> _lvalue;
    glm::vec3 _rvalue;
};

class Mat3MulVec2 : public Vec2 {
public:
    Mat3MulVec2(const sp<Mat3>& lvalue, const sp<Vec2>& rvalue)
        : _lvalue(lvalue), _rvalue(rvalue) {
    }

    virtual V2 val() override {
        const glm::mat3 lvalue = _lvalue->val().mat<glm::mat3>();
        const V2 rvalue = _rvalue->val();
        const glm::vec3 result = lvalue * glm::vec3(rvalue.x(), rvalue.y(), 1.0f);
        DCHECK(result.z != 0, "Division by zero");
        return V2(result.x / result.z, result.y / result.z);
    }

private:
    sp<Mat3> _lvalue;
    sp<Vec2> _rvalue;
};

class Mat3MulV2 : public Vec2 {
public:
    Mat3MulV2(const sp<Mat3>& lvalue, const V2& rvalue)
        : _lvalue(lvalue), _rvalue(rvalue.x(), rvalue.y(), 1.0f) {
    }

    virtual V2 val() override {
        const glm::mat3 lvalue = _lvalue->val().mat<glm::mat3>();
        const glm::vec3 result = lvalue * _rvalue;
        DCHECK(result.z != 0, "Division by zero");
        return V2(result.x / result.z, result.y / result.z);
    }

private:
    sp<Mat3> _lvalue;
    glm::vec3 _rvalue;
};

}

sp<Mat3> Mat3Util::create(const sp<Vec3>& t, const sp<Vec3>& b, const sp<Vec3>& n)
{
    return sp<Mat3Impl>::make(t, b, n);
}

sp<Mat3> Mat3Util::create(const V3& t, const V3& b, const V3& n)
{
    return sp<Mat3Impl>::make(t, b, n);
}

sp<Mat3> Mat3Util::mul(const sp<Mat3>& lvalue, const sp<Mat3>& rvalue)
{
    return sp<Mat3MulMat3>::make(lvalue, rvalue);
}

sp<Vec3> Mat3Util::mul(const sp<Mat3>& lvalue, const sp<Vec3>& rvalue)
{
    return sp<Mat3MulVec3>::make(lvalue, rvalue);
}

sp<Vec3> Mat3Util::mul(const sp<Mat3>& lvalue, const V3& rvalue)
{
    return sp<Mat3MulV3>::make(lvalue, rvalue);
}

sp<Vec2> Mat3Util::mul(const sp<Mat3>& lvalue, const sp<Vec2>& rvalue)
{
    return sp<Mat3MulVec2>::make(lvalue, rvalue);
}

sp<Vec2> Mat3Util::mul(const sp<Mat3>& lvalue, const V2& rvalue)
{
    return sp<Mat3MulV2>::make(lvalue, rvalue);
}

void Mat3Util::fix(const sp<Mat3>& self)
{
    ensureImpl(self)->fix();
}

sp<Mat3Impl> Mat3Util::ensureImpl(const sp<Mat3>& self)
{
    const sp<Mat3Impl> impl = self.as<Mat3Impl>();
    DCHECK(impl, "This Vec3 object is not a Mat3Impl instance");
    return impl;
}

template<> ARK_API const sp<Mat3> Null::ptr()
{
    return Ark::instance().obtain<Mat3Impl>();
}

}
