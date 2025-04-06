#pragma once

#include <functional>

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
class Emitter;
class RenderTarget;
class GraphicsBufferAllocator;
class GraphicsContext;
class Layer;
class MaterialBundle;
class Mesh;
class Model;
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
class Recycler;
class Resource;
class RenderEngineContext;
class RenderController;
class DrawingContextComposer;
class RenderEngine;
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

typedef std::function<void(GraphicsContext&)> ResourceRecycleFunc;

}
