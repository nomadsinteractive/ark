#ifndef ARK_GRAPHICS_BASE_BITMAP_H_
#define ARK_GRAPHICS_BASE_BITMAP_H_

#include "core/base/api.h"
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

    class Util {
    public:
        template<typename T> static void rotate(T* data, uint32_t width, uint32_t height, uint8_t channels, int32_t degrees) {
            DCHECK(width == height, "Can only rotate squares");
            if(degrees == 90)
                hvflip<T>(data, width, height, channels);
            else if(degrees == 180)
                vflip(data, width, height, channels);
            else if(degrees == 270) {
                hflip<T>(data, width, height, channels);
                hvflip<T>(data, width, height, channels);
            } else
                DCHECK(degrees == 0, "Unsupported rotation angle: %d", degrees);
        }

        template<typename T> static void hflip(T* data, uint32_t width, uint32_t height, uint8_t channels) {
            for(uint32_t i = 0; i < height; ++i)
                for(uint32_t j = 0; j < width / 2; ++j) {
                    T* r1 = data + (width * i + j) * channels;
                    T* r2 = data + (width * i + width - j - 1) * channels;
                    for(uint8_t k = 0; k < channels; ++k)
                        swap(r1[k], r2[k]);
                }
        }

        template<typename T> static void vflip(T* data, uint32_t width, uint32_t height, uint8_t channels) {
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

        template<typename T> static void hvflip(T* data, uint32_t width, uint32_t height, uint8_t channels) {
            for(uint32_t i = 0; i < height; ++i)
                for(uint32_t j = i + 1; j < width; ++j) {
                    T* r1 = data + (width * i + j) * channels;
                    T* r2 = data + (width * j + i) * channels;
                    for(uint8_t k = 0; k < channels; ++k)
                        swap(r1[k], r2[k]);
                }
        }

        template<typename T> static void swap(T& a1, T& a2) {
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
        sp<BitmapBundle> _bitmap_bundle;
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
