#include "sdl3/impl/buffer/buffer_sdl3_gpu.h"

#include "core/util/uploader_type.h"

#include "sdl3/base/context_sdl3_gpu.h"

namespace ark::plugin::sdl3 {

Buffer_SDL3_GPU::Buffer_SDL3_GPU(const SDL_GPUBufferUsageFlags usageFlags)
    : _usage_flags(usageFlags)
{
}

uint64_t Buffer_SDL3_GPU::id()
{
    return reinterpret_cast<uint64_t>(_buffer);
}

void Buffer_SDL3_GPU::upload(GraphicsContext& graphicsContext)
{
}

ResourceRecycleFunc Buffer_SDL3_GPU::recycle()
{
    SDL_GPUBuffer* buffer = _buffer;
    _buffer = nullptr;

    return [buffer] (GraphicsContext& graphicsContext) {
        SDL_ReleaseGPUBuffer(ensureGPUDevice(graphicsContext), buffer);
    };
}

void Buffer_SDL3_GPU::uploadBuffer(GraphicsContext& graphicsContext, Uploader& input)
{
    SDL_GPUDevice* gpuDevice = ensureGPUDevice(graphicsContext);

    const Uint32 inputSize = input.size();
    const SDL_GPUTransferBufferCreateInfo transferBufferCreateInfo{SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD, inputSize};
    SDL_GPUTransferBuffer* uploadTransferBuffer = SDL_CreateGPUTransferBuffer(gpuDevice, &transferBufferCreateInfo);

    void* transferData = SDL_MapGPUTransferBuffer(gpuDevice, uploadTransferBuffer, false);
    UploaderType::writeTo(input, transferData);
    SDL_UnmapGPUTransferBuffer(gpuDevice, uploadTransferBuffer);

    SDL_GPUCommandBuffer* uploadCmdBuf = SDL_AcquireGPUCommandBuffer(gpuDevice);
    SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(uploadCmdBuf);

    const SDL_GPUTransferBufferLocation transferBufferLocation{uploadTransferBuffer, 0};
    const SDL_GPUBufferRegion bufferRegion{_buffer, 0, inputSize};
    SDL_UploadToGPUBuffer(copyPass, &transferBufferLocation, &bufferRegion, false);

    SDL_EndGPUCopyPass(copyPass);
    SDL_SubmitGPUCommandBuffer(uploadCmdBuf);
    SDL_ReleaseGPUTransferBuffer(gpuDevice, uploadTransferBuffer);
}

void Buffer_SDL3_GPU::downloadBuffer(GraphicsContext& graphicsContext, size_t offset, size_t size, void* ptr)
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

SDL_GPUBuffer* Buffer_SDL3_GPU::buffer() const
{
    return _buffer;
}

}
