#pragma once

#include "core/forwarding.h"

namespace ark {

class Animation;
class Atlas;
class Attribute;
class BitmapBundle;
class Buffer;
class ComputeContext;
class DrawDecoratorFactory;
class DrawingBuffer;
class DrawingContext;
class RenderTarget;
class GraphicsBufferAllocator;
class GraphicsContext;
class Layer;
class MaterialBundle;
class Mesh;
class Model;
class ModelImporter;
class ModelLoader;
class ModelBundle;
class Node;
class Pipeline;
class PipelineBindings;
class PipelineContext;
class PipelineDrawCommand;
class PipelineFactory;
class PipelineLayout;
class PipelineDescriptor;
class PipelineBuildingContext;
class Recyclable;
class Recycler;
class Resource;
class RenderBackendInfo;
class RenderController;
class DrawingContextComposer;
class RenderBackend;
class RendererFactory;
class ResourceLoaderContext;
class Shader;
class Snippet;
class ShaderPreprocessor;
class SnippetFactory;
class Texture;
class TextureBundle;
class TexturePacker;
class VertexWriter;
class Vertices;
class Uniform;

//[[script::bindings::auto]]
typedef Importer<Atlas> AtlasImporter;

// Capped at 1 for now: per-frame GPU resources (uniform/vertex buffers updated via RenderController::onDrawFrame)
// are single-buffered and written in place, so allowing more than one frame in flight would let the CPU
// overwrite memory the GPU is still reading. Raising this to 2+ to actually overlap CPU/GPU work additionally
// requires per-frame-in-flight ring-buffering of those dynamically-updated resources.
constexpr uint32_t kMaxFramesInFlight = 1;

}
