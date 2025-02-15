#include "sdl3/impl/texture/texture_sdl3_gpu.h"

#include "core/inf/array.h"
#include "graphics/base/bitmap.h"

namespace ark::plugin::sdl3 {

TextureSDL3_GPU::TextureSDL3_GPU(const uint32_t width, const uint32_t height, sp<Texture::Parameters> parameters)
    : Delegate(parameters->_type), _width(width), _height(height), _parameters(std::move(parameters)), _texture(nullptr), _sampler(nullptr)
{
}

uint64_t TextureSDL3_GPU::id()
{
    return reinterpret_cast<uint64_t>(_texture);
}

void TextureSDL3_GPU::upload(GraphicsContext& graphicsContext, const sp<Texture::Uploader>& uploader)
{
    if(uploader)
    {
        if(_texture)
            uploader->update(graphicsContext, *this);
        else
            uploader->initialize(graphicsContext, *this);
    }
}

ResourceRecycleFunc TextureSDL3_GPU::recycle()
{
    SDL_GPUTexture* texture = _texture;
    SDL_GPUSampler* sampler = _sampler;

    _texture = nullptr;
    _sampler = nullptr;

    return [texture, sampler] (GraphicsContext& graphicsContext) {
        SDL_GPUDevice* gpuDevice = ensureGPUDevice(graphicsContext);
        if(texture)
            SDL_ReleaseGPUTexture(gpuDevice, texture);
        if(sampler)
            SDL_ReleaseGPUSampler(gpuDevice, sampler);
    };
}

void TextureSDL3_GPU::clear(GraphicsContext& graphicsContext)
{
}

bool TextureSDL3_GPU::download(GraphicsContext& graphicsContext, Bitmap& bitmap)
{
    return false;
}

void TextureSDL3_GPU::uploadBitmap(GraphicsContext& graphicsContext, const Bitmap& bitmap, const Vector<sp<ByteArray>>& imagedata)
{
    if(!_texture)
        _texture = createTexture(graphicsContext);

    SDL_GPUDevice* gpuDevice = ensureGPUDevice(graphicsContext);
    SDL_GPUCommandBuffer* uploadCmdBuf = SDL_AcquireGPUCommandBuffer(gpuDevice);
    SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(uploadCmdBuf);

    const Uint32 bitmapSize = bitmap.rowBytes() * bitmap.height();
    const SDL_GPUTransferBufferCreateInfo transferBufferCreateInfo{SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD, bitmapSize};
    SDL_GPUTransferBuffer* textureTransferBuffer = SDL_CreateGPUTransferBuffer(gpuDevice, &transferBufferCreateInfo);
    
    void* transferData = SDL_MapGPUTransferBuffer(gpuDevice, textureTransferBuffer, false);
    DASSERT(imagedata.at(0)->size() <= bitmapSize);
    memcpy(transferData, imagedata.at(0)->buf(), bitmapSize);
    SDL_UnmapGPUTransferBuffer(gpuDevice, textureTransferBuffer);

    const SDL_GPUTextureTransferInfo textureTransferInfo{textureTransferBuffer, 0, bitmap.rowBytes(), 0};
    const SDL_GPUTextureRegion textureRegion{_texture, 0, 0, 0, 0, 0, bitmap.width(), bitmap.height(), 1};
    SDL_UploadToGPUTexture(copyPass, &textureTransferInfo, &textureRegion, false);

    SDL_EndGPUCopyPass(copyPass);
    SDL_SubmitGPUCommandBuffer(uploadCmdBuf);
    SDL_ReleaseGPUTransferBuffer(gpuDevice, textureTransferBuffer);
}

SDL_GPUTexture* TextureSDL3_GPU::texture() const
{
    return _texture;
}

SDL_GPUSampler* TextureSDL3_GPU::ensureSampler(SDL_GPUDevice *gpuDevice)
{
    if(!_sampler)
    {
        const SDL_GPUSamplerCreateInfo samplerCreateInfo = {
            SDL_GPU_FILTER_NEAREST,
            SDL_GPU_FILTER_NEAREST
        };
        _sampler = SDL_CreateGPUSampler(gpuDevice, &samplerCreateInfo);
    }
    return _sampler;
}

SDL_GPUTexture* TextureSDL3_GPU::createTexture(GraphicsContext& graphicsContext) const
{
    SDL_GPUDevice* gpuDevice = ensureGPUDevice(graphicsContext);
    const Texture::Parameters& parameters = *_parameters;
    const SDL_GPUTextureCreateInfo textureCreateInfo{parameters._type == Texture::TYPE_2D ? SDL_GPU_TEXTURETYPE_2D : SDL_GPU_TEXTURETYPE_CUBE, SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM, SDL_GPU_TEXTUREUSAGE_SAMPLER, _width, _height, 1, 1};
    return SDL_CreateGPUTexture(gpuDevice, &textureCreateInfo);
}

}
