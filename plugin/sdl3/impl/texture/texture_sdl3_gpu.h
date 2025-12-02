#pragma once

#include <SDL3/SDL.h>

#include "renderer/base/texture.h"

namespace ark::plugin::sdl3 {

class TextureSDL3_GPU final : public Texture::Delegate {
public:
    TextureSDL3_GPU(uint32_t width, uint32_t height, sp<Texture::Parameters> parameters);

    uint64_t id() override;

    void upload(GraphicsContext& graphicsContext, const sp<Texture::Uploader>& uploader) override;
    ResourceRecycleFunc recycle() override;
    void clear(GraphicsContext& graphicsContext) override;
    bool download(GraphicsContext& graphicsContext, Bitmap& bitmap) override;
    void uploadBitmap(GraphicsContext& graphicsContext, const Bitmap& bitmap, const std::vector<sp<ByteArray>>& imagedata) override;

    SDL_GPUTexture* texture() const;
    SDL_GPUTextureFormat textureFormat() const;

    SDL_GPUSampler* ensureSampler(SDL_GPUDevice *gpuDevice);

private:
    uint32_t _width;
    uint32_t _height;
    sp<Texture::Parameters> _parameters;
    SDL_GPUTextureFormat _texture_format;

    SDL_GPUTexture* _texture;
    SDL_GPUSampler* _sampler;
};

}
