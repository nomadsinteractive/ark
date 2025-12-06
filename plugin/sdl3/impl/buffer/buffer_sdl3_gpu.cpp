#include "sdl3/impl/buffer/buffer_sdl3_gpu.h"

#include "core/inf/writable.h"
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

class WritableSDL3_GPU_Buffer final : public Writable {
public:
    WritableSDL3_GPU_Buffer(SDL_GPUDevice* gpuDevice, SDL_GPUBuffer* buffer)
        : _gpu_device(gpuDevice), _buffer(buffer), _upload_command_buffer(SDL_AcquireGPUCommandBuffer(_gpu_device)), _copy_pass(SDL_BeginGPUCopyPass(_upload_command_buffer))
    {
    }

    ~WritableSDL3_GPU_Buffer() override
    {
        SDL_EndGPUCopyPass(_copy_pass);
        SDL_SubmitGPUCommandBuffer(_upload_command_buffer);

        for(SDL_GPUTransferBuffer* i : _transfer_buffers)
            SDL_ReleaseGPUTransferBuffer(_gpu_device, i);
    }

    uint32_t write(const void* buffer, const uint32_t size, const uint32_t offset) override
    {
        const SDL_GPUTransferBufferCreateInfo transferBufferCreateInfo = {SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD, size};
        SDL_GPUTransferBuffer* uploadTransferBuffer = SDL_CreateGPUTransferBuffer(_gpu_device, &transferBufferCreateInfo);
        void* transferData = SDL_MapGPUTransferBuffer(_gpu_device, uploadTransferBuffer, false);
        memcpy(transferData, buffer, size);
        SDL_UnmapGPUTransferBuffer(_gpu_device, uploadTransferBuffer);

        const SDL_GPUTransferBufferLocation transferBufferLocation = {uploadTransferBuffer, 0};
        const SDL_GPUBufferRegion bufferRegion = {_buffer, offset, size};
        SDL_UploadToGPUBuffer(_copy_pass, &transferBufferLocation, &bufferRegion, false);
        _transfer_buffers.push_back(uploadTransferBuffer);
        return size;
    }

private:
    SDL_GPUDevice* _gpu_device;
    SDL_GPUBuffer* _buffer;
    SDL_GPUCommandBuffer* _upload_command_buffer;
    SDL_GPUCopyPass* _copy_pass;

    Vector<SDL_GPUTransferBuffer*> _transfer_buffers;
};

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

    WritableSDL3_GPU_Buffer writable(gpuDevice, _buffer);
    for(const auto& [k, v] : UploaderType::record(uploader))
        writable.write(v.data(), v.size(), k);
}

void BufferSDL3_GPU::downloadBuffer(GraphicsContext& graphicsContext, const size_t offset, const size_t size, void* ptr)
{
    SDL_GPUDevice* gpuDevice = ensureGPUDevice(graphicsContext);

    const SDL_GPUTransferBufferCreateInfo transferBufferCreateInfo{SDL_GPU_TRANSFERBUFFERUSAGE_DOWNLOAD, static_cast<Uint32>(size)};
    SDL_GPUTransferBuffer* downloadTransferBuffer = SDL_CreateGPUTransferBuffer(gpuDevice, &transferBufferCreateInfo);

    SDL_GPUCommandBuffer* downloadCmdBuf = SDL_AcquireGPUCommandBuffer(gpuDevice);
    SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(downloadCmdBuf);

    const SDL_GPUBufferRegion bufferRegion = {_buffer, static_cast<Uint32>(offset), static_cast<Uint32>(size)};
    const SDL_GPUTransferBufferLocation transferBufferLocation = {downloadTransferBuffer, 0};
    SDL_DownloadFromGPUBuffer(copyPass, &bufferRegion, &transferBufferLocation);
    SDL_EndGPUCopyPass(copyPass);

    SDL_GPUFence* fence = SDL_SubmitGPUCommandBufferAndAcquireFence(downloadCmdBuf);
    SDL_WaitForGPUFences(gpuDevice, true, &fence, 1);
    SDL_ReleaseGPUFence(gpuDevice, fence);

    const Uint8* downloadedData = static_cast<const Uint8*>(SDL_MapGPUTransferBuffer(gpuDevice, downloadTransferBuffer, false));
    memcpy(ptr, downloadedData, size);
    SDL_UnmapGPUTransferBuffer(gpuDevice, downloadTransferBuffer);

    SDL_ReleaseGPUTransferBuffer(gpuDevice, downloadTransferBuffer);
}

}
