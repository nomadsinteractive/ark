#include "sdl3/impl/buffer/buffer_sdl3_gpu.h"

#include "core/util/uploader_type.h"

#include "sdl3/base/context_sdl3_gpu.h"

namespace ark::plugin::sdl3 {

BufferSDL3_GPU::BufferSDL3_GPU(const SDL_GPUBufferUsageFlags usageFlags)
    : _usage_flags(usageFlags), _buffer(nullptr), _buffer_size(0)
{
}

uint64_t BufferSDL3_GPU::id()
{
    return reinterpret_cast<uint64_t>(_buffer);
}

void BufferSDL3_GPU::upload(GraphicsContext& graphicsContext)
{
}

ResourceRecycleFunc BufferSDL3_GPU::recycle()
{
    SDL_GPUBuffer* buffer = _buffer;
    _buffer = nullptr;
    _buffer_size = 0;

    return [buffer] (GraphicsContext& graphicsContext) {
        SDL_ReleaseGPUBuffer(ensureGPUDevice(graphicsContext), buffer);
    };
}

void BufferSDL3_GPU::uploadBuffer(GraphicsContext& graphicsContext, Uploader& uploader)
{
    SDL_GPUDevice* gpuDevice = ensureGPUDevice(graphicsContext);

    const Uint32 bufferSize = std::max<Uint32>(uploader.size(), 16);
    if(!_buffer || bufferSize > _buffer_size)
    {
        if(_buffer)
            SDL_ReleaseGPUBuffer(gpuDevice, _buffer);

        const SDL_GPUBufferCreateInfo bufferCreateInfo = {_usage_flags, bufferSize};
        _buffer = SDL_CreateGPUBuffer(gpuDevice, &bufferCreateInfo);
        _buffer_size = bufferSize;
    }

    const SDL_GPUTransferBufferCreateInfo transferBufferCreateInfo{SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD, bufferSize};
    SDL_GPUTransferBuffer* uploadTransferBuffer = SDL_CreateGPUTransferBuffer(gpuDevice, &transferBufferCreateInfo);

    void* transferData = SDL_MapGPUTransferBuffer(gpuDevice, uploadTransferBuffer, false);
    if(uploader.size() < bufferSize)
        memset(transferData, 0, bufferSize);
    else
        UploaderType::writeTo(uploader, transferData);
    SDL_UnmapGPUTransferBuffer(gpuDevice, uploadTransferBuffer);

    SDL_GPUCommandBuffer* uploadCmdBuf = SDL_AcquireGPUCommandBuffer(gpuDevice);
    SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(uploadCmdBuf);

    const SDL_GPUTransferBufferLocation transferBufferLocation{uploadTransferBuffer, 0};
    const SDL_GPUBufferRegion bufferRegion{_buffer, 0, bufferSize};
    SDL_UploadToGPUBuffer(copyPass, &transferBufferLocation, &bufferRegion, false);

    SDL_EndGPUCopyPass(copyPass);
    SDL_SubmitGPUCommandBuffer(uploadCmdBuf);
    SDL_ReleaseGPUTransferBuffer(gpuDevice, uploadTransferBuffer);
}

void BufferSDL3_GPU::downloadBuffer(GraphicsContext& graphicsContext, size_t offset, size_t size, void* ptr)
{
    SDL_GPUDevice* gpuDevice = ensureGPUDevice(graphicsContext);

    const SDL_GPUTransferBufferCreateInfo transferBufferCreateInfo{SDL_GPU_TRANSFERBUFFERUSAGE_DOWNLOAD, static_cast<Uint32>(size)};
    SDL_GPUTransferBuffer* downloadTransferBuffer = SDL_CreateGPUTransferBuffer(gpuDevice, &transferBufferCreateInfo);

    SDL_GPUCommandBuffer* uploadCmdBuf = SDL_AcquireGPUCommandBuffer(gpuDevice);
    SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(uploadCmdBuf);

    const SDL_GPUBufferRegion bufferRegion{_buffer, static_cast<Uint32>(offset), static_cast<Uint32>(size)};
    const SDL_GPUTransferBufferLocation transferBufferLocation{downloadTransferBuffer, 0};
    SDL_DownloadFromGPUBuffer(copyPass, &bufferRegion, &transferBufferLocation);
    SDL_EndGPUCopyPass(copyPass);
    SDL_SubmitGPUCommandBuffer(uploadCmdBuf);

    const Uint8* downloadedData = static_cast<const Uint8*>(SDL_MapGPUTransferBuffer(gpuDevice, downloadTransferBuffer, false));
    memcpy(ptr, downloadedData, size);
    SDL_UnmapGPUTransferBuffer(gpuDevice, downloadTransferBuffer);

    SDL_ReleaseGPUTransferBuffer(gpuDevice, downloadTransferBuffer);
}

}
