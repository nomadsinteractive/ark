#include "sdl3/impl/texture/texture_sdl3_gpu.h"

#include "core/inf/array.h"
#include "graphics/base/bitmap.h"

namespace ark::plugin::sdl3 {

namespace {

SDL_GPUTextureFormat toChannelFormat(const SDL_GPUTextureFormat* channelFormat, const uint32_t depths, const Texture::Format format)
{
    if(depths == 1)
    {
        CHECK(!(format & Texture::FORMAT_FLOAT), "Component size one doesn't support float format");
        return channelFormat[0];
    }
    if(depths == 2)
    {
        if(format & Texture::FORMAT_FLOAT)
            return channelFormat[4];
        return format & Texture::FORMAT_SIGNED ? channelFormat[3] : channelFormat[2];
    }
    DCHECK(depths == 4, "Unsupported color-depth: %d", depths * 8);
    if(format & Texture::FORMAT_FLOAT)
        return channelFormat[5];
    return format & Texture::FORMAT_SIGNED ? channelFormat[7] : channelFormat[6];
}

SDL_GPUTextureFormat toTextureFormat(const Bitmap& bitmap, const Texture::Format format, const Texture::Usage usage)
{
    if(usage.contains(Texture::USAGE_DEPTH_ATTACHMENT))
        return SDL_GPU_TEXTUREFORMAT_D32_FLOAT;

    constexpr SDL_GPUTextureFormat sdlFormats[] = {
        SDL_GPU_TEXTUREFORMAT_R8_UNORM, SDL_GPU_TEXTUREFORMAT_R8_SNORM, SDL_GPU_TEXTUREFORMAT_R16_UNORM, SDL_GPU_TEXTUREFORMAT_R16_SNORM, SDL_GPU_TEXTUREFORMAT_R16_FLOAT, SDL_GPU_TEXTUREFORMAT_R32_FLOAT, SDL_GPU_TEXTUREFORMAT_R32_UINT, SDL_GPU_TEXTUREFORMAT_R32_INT,
        SDL_GPU_TEXTUREFORMAT_R8G8_UNORM, SDL_GPU_TEXTUREFORMAT_R8G8_SNORM, SDL_GPU_TEXTUREFORMAT_R16G16_UNORM, SDL_GPU_TEXTUREFORMAT_R16G16_SNORM, SDL_GPU_TEXTUREFORMAT_R16G16_FLOAT, SDL_GPU_TEXTUREFORMAT_R32G32_FLOAT, SDL_GPU_TEXTUREFORMAT_R32G32_UINT, SDL_GPU_TEXTUREFORMAT_R32G32_INT,
        SDL_GPU_TEXTUREFORMAT_R8G8_UNORM, SDL_GPU_TEXTUREFORMAT_R8G8_SNORM, SDL_GPU_TEXTUREFORMAT_R16G16_UNORM, SDL_GPU_TEXTUREFORMAT_R16G16_SNORM, SDL_GPU_TEXTUREFORMAT_R16G16_FLOAT, SDL_GPU_TEXTUREFORMAT_R32G32_FLOAT, SDL_GPU_TEXTUREFORMAT_R32G32_UINT, SDL_GPU_TEXTUREFORMAT_R32G32_INT,
        SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM, SDL_GPU_TEXTUREFORMAT_R8G8B8A8_SNORM, SDL_GPU_TEXTUREFORMAT_R16G16B16A16_UNORM, SDL_GPU_TEXTUREFORMAT_R16G16B16A16_SNORM, SDL_GPU_TEXTUREFORMAT_R16G16B16A16_FLOAT, SDL_GPU_TEXTUREFORMAT_R32G32B32A32_FLOAT, SDL_GPU_TEXTUREFORMAT_R32G32B32A32_UINT, SDL_GPU_TEXTUREFORMAT_R32G32B32A32_INT
    };
    CHECK(bitmap.channels() != 3, "3 channels textures are not supported");
    CHECK(!(format & Texture::FORMAT_SIGNED && format & Texture::FORMAT_FLOAT), "FORMAT_SIGNED format can not combined with FORMAT_FLOAT");
    const uint32_t channel8 = (bitmap.channels() - 1) * 8;
    return toChannelFormat(sdlFormats + channel8, bitmap.depth(), format);
}

SDL_GPUTextureUsageFlags toTextureUsageFlags(const Texture::Usage usage)
{
    if(usage == Texture::USAGE_AUTO)
        return SDL_GPU_TEXTUREUSAGE_SAMPLER;

    SDL_GPUTextureUsageFlags flags = 0;
    if(usage.contains(Texture::USAGE_DEPTH_STENCIL_ATTACHMENT))
        flags |= SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET;
    if(usage.contains(Texture::USAGE_COLOR_ATTACHMENT))
        flags |= SDL_GPU_TEXTUREUSAGE_COLOR_TARGET;
    if(usage.contains(Texture::USAGE_SAMPLER))
        flags |= SDL_GPU_TEXTUREUSAGE_SAMPLER;
    if(usage.contains(Texture::USAGE_STORAGE))
        flags |= SDL_GPU_TEXTUREUSAGE_GRAPHICS_STORAGE_READ | SDL_GPU_TEXTUREUSAGE_COMPUTE_STORAGE_READ | SDL_GPU_TEXTUREUSAGE_COMPUTE_STORAGE_WRITE;
    return flags;
}

SDL_GPUFilter toFilter(const Texture::Filter filter)
{
    return filter == Texture::FILTER_NEAREST ? SDL_GPU_FILTER_NEAREST : SDL_GPU_FILTER_LINEAR;
}

SDL_GPUTexture* createTexture(GraphicsContext& graphicsContext, const Texture::Parameters& parameters, const SDL_GPUTextureFormat textureFormat, uint32_t width, uint32_t height)
{
    SDL_GPUDevice* gpuDevice = ensureGPUDevice(graphicsContext);
    const SDL_GPUTextureCreateInfo textureCreateInfo{parameters._type == Texture::TYPE_2D ? SDL_GPU_TEXTURETYPE_2D : SDL_GPU_TEXTURETYPE_CUBE, textureFormat, toTextureUsageFlags(parameters._usage), width, height, 1, 1};
    return SDL_CreateGPUTexture(gpuDevice, &textureCreateInfo);
}

}

TextureSDL3_GPU::TextureSDL3_GPU(const uint32_t width, const uint32_t height, sp<Texture::Parameters> parameters)
    : Delegate(parameters->_type), _width(width), _height(height), _parameters(std::move(parameters)), _texture_format(SDL_GPU_TEXTUREFORMAT_INVALID), _texture(nullptr), _sampler(nullptr)
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
    {
        _texture_format = toTextureFormat(bitmap, _parameters->_format, _parameters->_usage);
        _texture = createTexture(graphicsContext, _parameters, _texture_format, _width, _height);
    }

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

    const SDL_GPUTextureTransferInfo textureTransferInfo{textureTransferBuffer, 0, bitmap.width(), bitmap.height()};
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

SDL_GPUTextureFormat TextureSDL3_GPU::textureFormat() const
{
    return _texture_format;
}

SDL_GPUSampler* TextureSDL3_GPU::ensureSampler(SDL_GPUDevice* gpuDevice)
{
    if(!_sampler)
    {
        const Texture::Parameters& parameters = _parameters;
        const SDL_GPUSamplerCreateInfo samplerCreateInfo = {
            toFilter(parameters._min_filter),
            toFilter(parameters._mag_filter)
        };
        _sampler = SDL_CreateGPUSampler(gpuDevice, &samplerCreateInfo);
    }
    return _sampler;
}

}
