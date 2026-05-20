#include "core/util/float_array_type.h"

namespace ark {

sp<FloatArray> FloatArrayType::derivative2D(const sp<FloatArray>& self, const size_t width, const size_t height)
{
    CHECK(self->length() == width * height, "Array length(%zu) != width(%zu) * height(%zu)", self->length(), width, height);

    sp<FloatArray> result = create(width * height);
    const float* src = self->buf();
    float* dst = result->buf();

    for(size_t y = 0; y < height; ++y)
        for(size_t x = 0; x < width; ++x)
        {
            const size_t i = y * width + x;
            dst[i] = src[i];
            if(x > 0)
                dst[i] -= src[i - 1];
            if(y > 0)
                dst[i] -= src[i - width];
            if(x > 0 && y > 0)
                dst[i] += src[i - width - 1];
        }

    return result;
}

sp<FloatArray> FloatArrayType::integral2D(const sp<FloatArray>& self, const size_t width, const size_t height)
{
    CHECK(self->length() == width * height, "Array length(%zu) != width(%zu) * height(%zu)", self->length(), width, height);

    sp<FloatArray> result = create(width * height);
    const float* src = self->buf();
    float* dst = result->buf();

    for(size_t y = 0; y < height; ++y)
        for(size_t x = 0; x < width; ++x)
        {
            const size_t i = y * width + x;
            dst[i] = src[i];
            if(x > 0)
                dst[i] += dst[i - 1];
            if(y > 0)
                dst[i] += dst[i - width];
            if(x > 0 && y > 0)
                dst[i] -= dst[i - width - 1];
        }

    return result;
}

}
