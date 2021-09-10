#ifndef ARK_RENDERER_BASE_FORWARDING_H_
#define ARK_RENDERER_BASE_FORWARDING_H_

#include "core/forwarding.h"

namespace ark {

class Animation;
class AnimationInput;
class Atlas;
class Attribute;
class Buffer;
class Characters;
class ComputeContext;
class DrawingBuffer;
class DrawingContext;
class Emitter;
class Framebuffer;
class GraphicsContext;
class Layer;
class MaterialBundle;
class Mesh;
class Model;
class ModelLoader;
class ModelBundle;
class Pipeline;
class PipelineFactory;
class PipelineInput;
class PipelineBindings;
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
class ShaderBindings;
class Snippet;
class ShaderPreprocessor;
class SharedBuffer;
class SnippetFactory;
class Texture;
class TextureBundle;
class TexturePacker;
class VertexStream;
class Vertices;
class Uniform;
class Uploader;

//[[script::bindings::auto]]
typedef Importer<Atlas> AtlasImporter;

}

#endif
