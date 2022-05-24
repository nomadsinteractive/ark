#ifndef ARK_GRAPHICS_BASE_BITMAP_H_
#define ARK_GRAPHICS_BASE_BITMAP_H_

#include <array>

#include "core/base/api.h"
#include "core/base/string.h"
#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "renderer/forwarding.h"

namespace ark {

class ARK_API Bitmap {
public:
//  [[script::bindings::auto]]
    Bitmap(uint32_t width, uint32_t height, uint32_t rowBytes, uint8_t channels, bool allocate);
    Bitmap(uint32_t width, uint32_t height, uint32_t rowBytes, uint8_t channels, const bytearray& bytes);
    ~Bitmap();

    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(Bitmap);

//  [[script::bindings::property]]
    uint32_t width() const;
//  [[script::bindings::property]]
    uint32_t height() const;
//  [[script::bindings::property]]
    uint8_t channels() const;
//  [[script::bindings::property]]
    uint32_t depth() const;

//  [[script::bindings::property]]
    uint32_t rowBytes() const;

//  [[script::bindings::property]]
    const sp<Array<uint8_t>>& bytes() const;

    uint8_t* at(uint32_t x, uint32_t y) const;

    Bitmap resize(uint32_t width, uint32_t height) const;
    Bitmap crop(uint32_t x, uint32_t y, uint32_t width, uint32_t height) const;

    void draw(void* buf, uint32_t width, uint32_t height, int32_t x, int32_t y, uint32_t stride);

    void dump(const String& filename) const;

    template<typename T> class Util {
    private:
        static std::array<T*, 4> toRotationSpots(T* data, uint32_t n, uint8_t channels, uint32_t x, uint32_t y) {
            return {data + (y * n + x) * channels, data + (x * n + y) * channels, data + ((n -  y) * n + x) * channels, data + (x * n + n - y) * channels};
        }

        static std::array<uint32_t, 4> toRotationSequences(const std::array<uint32_t, 4>& seq, int32_t rotation) {
            switch(rotation % 4) {
            case 0:
                return seq;
            case 1:
                return {seq[3], seq[0], seq[1], seq[2]};
            case 2:
                return {seq[2], seq[3], seq[0], seq[1]};
            case 3:
                return {seq[1], seq[2], seq[3], seq[0]};
            }
        }

    public:
        static void rotate(T* data, uint32_t width, uint32_t height, uint8_t channels, int32_t degrees) {
            DCHECK(width == height, "Can only rotate squares");
            if(degrees == 90)
                hvflip(data, width, height, channels);
            else if(degrees == 180)
                vflip(data, width, height, channels);
            else if(degrees == 270) {
                hflip(data, width, height, channels);
                hvflip(data, width, height, channels);
            } else
                DCHECK(degrees == 0, "Unsupported rotation angle: %d", degrees);
        }

        static void hflip(T* data, uint32_t width, uint32_t height, uint8_t channels) {
            for(uint32_t i = 0; i < height; ++i)
                for(uint32_t j = 0; j < width / 2; ++j) {
                    T* r1 = data + (width * i + j) * channels;
                    T* r2 = data + (width * i + width - j - 1) * channels;
                    for(uint8_t k = 0; k < channels; ++k)
                        swap(r1[k], r2[k]);
                }
        }

        static void vflip(T* data, uint32_t width, uint32_t height, uint8_t channels) {
            for(uint32_t i = 0; i < height / 2; ++i) {
                T* r1 = data + width * i * channels;
                T* r2 = data + width * (height - i - 1) * channels;
                if(r1 != r2)
                    for(uint32_t j = 0; j < width; ++j) {
                        T* c1 = r1 + j * channels;
                        T* c2 = r2 + j * channels;
                        for(uint8_t k = 0; k < channels; ++k)
                            swap(c1[k], c2[k]);
                    }
            }
        }

        static void hvflip(T* data, uint32_t width, uint32_t height, uint8_t channels) {
            for(uint32_t i = 0; i < height; ++i)
                for(uint32_t j = i + 1; j < width; ++j) {
                    T* r1 = data + (width * i + j) * channels;
                    T* r2 = data + (width * j + i) * channels;
                    for(uint8_t k = 0; k < channels; ++k)
                        swap(r1[k], r2[k]);
                }
        }

        static void swap(T& a1, T& a2) {
            T tmp = a1;
            a1 = a2;
            a2 = tmp;
        }
    };

//  [[plugin::resource-loader]]
    class BUILDER : public Builder<Bitmap> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext);

        virtual sp<Bitmap> build(const Scope& args) override;

    private:
        sp<Builder<String>> _src;
        sp<BitmapLoaderBundle> _bitmap_bundle;
    };

//  [[plugin::resource-loader::by-value]]
    class VALUE_BUILDER : public Builder<Bitmap> {
    public:
        VALUE_BUILDER(BeanFactory& factory, const String& src, const sp<ResourceLoaderContext>& resourceLoaderContext);

        virtual sp<Bitmap> build(const Scope& args) override;

    private:
        String _src;
        sp<BitmapLoaderBundle> _bitmap_bundle;
    };

private:
    uint32_t _width;
    uint32_t _height;
    uint32_t _row_bytes;
    uint8_t _channels;

    bytearray _bytes;
};

}


#endif
