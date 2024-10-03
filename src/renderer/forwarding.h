#pragma once

#include <functional>

#include "core/forwarding.h"

namespace ark {

class Animation;
class AnimationUploader;
class Atlas;
class Attribute;
class BitmapBundle;
class Buffer;
class ComputeContext;
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
class PipelineFactory;
class PipelineInput;
class PipelineDescriptor;
class PipelineBuildingContext;
class PipelineLayout;
class Recycler;
class Resource;
class RenderEngineContext;
class RenderController;
class RenderCommandComposer;
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
