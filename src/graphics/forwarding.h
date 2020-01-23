#ifndef ARK_GRAPHICS_FORWARDING_H_
#define ARK_GRAPHICS_FORWARDING_H_

#include "core/forwarding.h"

namespace ark {

class Alphabet;
class Bitmap;
class Block;
class Bounds;
class Camera;
class CharacterMaker;
class CharacterMapper;
class Color;
class Font;
class Frame;
class BitmapBundle;
class Scrollable;
class RenderLayer;
class Mat3Impl;
class Model;
class RenderLayer;
class LayerContext;
class Varyings;
class RendererMaker;
class Transform;
class Transformable;
class Rect;
class Renderable;
class RenderCommand;
class RenderCommandPipeline;
class RenderObject;
class RenderView;
class Layer;
class RenderRequest;
class RenderablePassive;
class Renderer;
class RendererGroup;
class Rotate;
class Size;
class SurfaceController;
class Tilemap;
class TilemapLayer;
class Tileset;
class Vec2Impl;
class Vec3Impl;
class Vec4Impl;
class V2;
class V3;
class V4;
class Vec2Util;
class Viewport;

template<uint32_t S> class Mat;

typedef sp<Bitmap> bitmap;

typedef Mat<3> M3;
typedef Mat<4> M4;

typedef Variable<M3> Mat3;
typedef Variable<M4> Mat4;

typedef Variable<V2> Vec2;
typedef Variable<V3> Vec3;
typedef Variable<V4> Vec4;

typedef Loader<Bitmap> BitmapLoader;

//[[script::bindings::auto]]
typedef Importer<Tilemap> TilemapImporter;
//[[script::bindings::auto]]
typedef Importer<Tileset> TilesetImporter;

typedef Vec2 Vec;
typedef V2 V;
typedef Vec2Util VecUtil;

const int32_t DIMENSIONS = 2;

}

#endif
