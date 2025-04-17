#pragma once

#include "core/forwarding.h"

namespace ark {

class Alphabet;
class RotationAxisTheta;
class Bitmap;
class Boundaries;
class Bounds;
class Camera;
class GlyphMaker;
class Color;
class DrawDecorator;
class RotationEuler;
class Font;
class Glyph;
class Layer;
class LayerContextSnapshot;
class Layout;
class LayoutParam;
class Mat2Impl;
class Mat3Impl;
class Mat4Impl;
class Material;
class MaterialTexture;
class Model;
class NamedHash;
class LayerContext;
class Varyings;
class RendererMaker;
class Transform;
class Quaternion;
class Renderable;
class RenderBatch;
class RenderCommand;
class RenderCommandPipeline;
class RenderLayer;
class RenderObject;
class RenderView;
class RenderRequest;
class Renderer;
class RenderGroup;
class RenderLayer;
class RenderLayerSnapshot;
class Rotation;
class Size;
class SurfaceController;
class Tile;
class Tilemap;
class TilemapLayer;
class Tileset;
class Text;
class Translation;
class Vec2Impl;
class Vec3Impl;
class Vec4Impl;
class V2;
class V3;
class V4;
class Viewport;

template<typename T> class RectT;
template<uint32_t S> class Mat;

typedef RectT<float> RectF;
typedef RectT<int32_t> RectI;
typedef RectF Rect;

typedef sp<Bitmap> bitmap;

typedef Mat<2> M2;
typedef Mat<3> M3;
typedef Mat<4> M4;

typedef std::array<int32_t, 2> V2i;
typedef std::array<int32_t, 3> V3i;
typedef std::array<int32_t, 4> V4i;

typedef Variable<M2> Mat2;
typedef Variable<M3> Mat3;
typedef Variable<M4> Mat4;

typedef Variable<V2> Vec2;
typedef Variable<V3> Vec3;
typedef Variable<V4> Vec4;

typedef VariableWrapper<M3> Mat3Wrapper;
typedef VariableWrapper<M4> Mat4Wrapper;

typedef Loader<sp<Bitmap>> BitmapLoader;
typedef LoaderBundle<sp<Bitmap>> BitmapLoaderBundle;

//[[script::bindings::auto]]
typedef Importer<Tileset> TilesetImporter;

}
